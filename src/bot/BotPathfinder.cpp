#include "BotPathfinder.h"
#include "worldedit/WorldEdit.h"
#include <algorithm>
#include <chrono>

namespace we::bot {

double distanceHeuristic(Vec3 const& a, Vec3 const& b) {
    double dx = a.x - b.x;
    double dy = (a.y - b.y) * 0.5;
    double dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

BotPathfinder::BotPathfinder(
    SimulatedPlayer*                     player,
    std::function<bool(BlockPos const&)> customBlockCheck
)
: mPlayer(player),
  mMovements(std::make_unique<Movements>(player, std::move(customBlockCheck))) {
    // Initialize path deviation tracking
    mLastProgressTick  = mCurrentTick;
    mLastProgressIndex = 0;
}


void BotPathfinder::setMovements(std::unique_ptr<Movements> movements) {
    mMovements = std::move(movements);
    resetPath("movements_updated");
}

ll::coro::CoroTask<ComputedPath>
BotPathfinder::getPathToAsync(Goal const& goal, std::chrono::milliseconds timeout) {
    if (timeout == std::chrono::milliseconds(0)) {
        timeout = thinkTimeout;
    }

    auto currentPos = BlockPos(getCurrentPosition());

    // Create a start move from current position
    auto startMove = std::make_unique<Move>(
        currentPos.x,
        currentPos.y,
        currentPos.z,
        (int)mMovements->countScaffoldingItems(),
        0.0
    );

    // Create a copy of the goal for AStar using the clone method
    auto goalCopy = goal.clone();

    // Create AStar instance
    auto astar = std::make_unique<AStar>(
        std::move(startMove),
        mMovements.get(),
        std::move(goalCopy),
        timeout,
        this->searchRadius
    );

    co_return co_await astar->computeAsync();
}

void BotPathfinder::stop() {
    mStopRequested = true;
    mIsMoving      = false;
    mIsMining      = false;
    mIsBuilding    = false;
    mIsExecuting   = false;

    resetPath("stop_requested", true);
    clearControlStates();
}

void BotPathfinder::reset(std::string const& reason) { resetPath(reason, true); }

void BotPathfinder::setExecutingPath(std::vector<std::unique_ptr<Move>> path) {
    // Set the computed path for execution
    mCurrentPath      = postProcessPath(std::move(path));
    mCurrentPathIndex = 0;
    mIsExecuting      = true;
    mIsMoving         = true;

    // Reset path deviation tracking for new path
    mLastProgressTick  = mCurrentTick;
    mLastProgressIndex = 0;

    if (debugPath) {
        std::vector<Vec3> points;
        for (auto const& move : mCurrentPath) {
            points.push_back(move->getVec3() + Vec3(0.0f, 0.5f, 0.0f));
        }
        // Create or update debug geometry
        mDebugPathGeo = WorldEdit::getInstance().getGeo().line(
            getDimension()->getDimensionId(),
            points,
            mce::Color::GREEN(),
            0.1f
        );
    }

    // Notify path updated
    if (mOnPathUpdated) {
        mOnPathUpdated(mCurrentPath);
    }
}

bool BotPathfinder::tick() {
    mCurrentTick++;
    if (!mIsExecuting || !mPlayer) return true;
    bool result = executeNextMove();
    if (!result) {
        mPlayer->simulateChat("I'm stuck!");
        stop();
    }
    return result;
}

bool BotPathfinder::executeNextMove() {
    auto const& path = getCurrentPath();
    if (path.empty() || mCurrentPathIndex >= path.size()) {
        mIsExecuting = false;
        mIsMining    = false;
        mIsBuilding  = false;
        clearControlStates();
        applyControlStatesToPlayer();
        return true;
    }

    Move& currentMove = *path[mCurrentPathIndex];
    Vec3  currentPos  = getCurrentPosition();
    Vec3  targetPos   = currentMove.getVec3();

    double distance = distanceHeuristic(currentPos, targetPos);
    auto   now      = mCurrentTick;

    // Check for path deviation - distance from expected path position
    if (distance > MAX_DEVIATION_DISTANCE) {
        mIsExecuting = false;
        mIsMining    = false;
        mIsBuilding  = false;
        clearControlStates();
        applyControlStatesToPlayer();
        return false; // Path deviation too large
    }

    // Check for progress timeout - stuck at same index for too long
    if (mCurrentPathIndex == mLastProgressIndex) {
        auto timeSinceProgress = now - mLastProgressTick;
        if (timeSinceProgress > PROGRESS_TIMEOUT) {
            mIsExecuting = false;
            mIsMining    = false;
            mIsBuilding  = false;
            clearControlStates();
            applyControlStatesToPlayer();
            return false; // Stuck for too long
        }
    } else {
        // Made progress, update tracking
        mLastProgressTick  = now;
        mLastProgressIndex = mCurrentPathIndex;
    }

    // Check if we've reached the current waypoint
    if (distance < 0.3 && !needsToBreakBlocks(currentMove)
        && !needsToPlaceBlocks(currentMove)) {
        mCurrentPathIndex++;
        return true;
    }

    // Handle block breaking operations
    if (needsToBreakBlocks(currentMove)) {
        mIsMining   = true; // Set mining flag
        mIsBuilding = false;

        bool allBroken = true;
        for (auto const& blockToBreak : currentMove.toBreak) {
            auto breakState = breakBlock(blockToBreak.pos);
            if (breakState == BreakBlockState::Fail) {
                allBroken = false;
                break;
            } else if (breakState == BreakBlockState::Breaking) {
                // Still breaking, continue with current operation
                clearControlStates();
                applyControlStatesToPlayer();
                return true;
            }
        }

        if (allBroken) {
            currentMove.toBreak.clear(); // Clear after processing
            mIsMining = false;           // No longer mining
        } else {
            clearControlStates();
            applyControlStatesToPlayer();
            return true;
        }
    }

    // Handle block placing operations
    if (needsToPlaceBlocks(currentMove)) {
        mIsBuilding = true; // Set building flag
        mIsMining   = false;

        bool allPlaced = true;
        for (auto const& blockToPlace : currentMove.toPlace) {
            if (blockToPlace.pos == BlockPos{currentPos}) {
                updateControlStateForPosition(targetPos);
                return true;
            }
            if (!placeBlock(blockToPlace.pos, getScaffoldingBlock())) {
                allPlaced = false;
                // Continue trying to place remaining blocks
                clearControlStates();
                applyControlStatesToPlayer();
                return true;
            }
        }

        if (allPlaced) {
            currentMove.toPlace.clear(); // Clear after processing
            mIsBuilding = false;         // No longer building
        } else {
            clearControlStates();
            applyControlStatesToPlayer();
            return true;
        }
    }

    // If no block operations are needed, clear the flags
    if (!needsToBreakBlocks(currentMove) && !needsToPlaceBlocks(currentMove)) {
        mIsMining   = false;
        mIsBuilding = false;
    }

    // Update movement controls based on current position and target
    updateControlStateForPosition(targetPos);
    return true;
}

void BotPathfinder::updateControlState(Move const& currentMove, Move const& nextMove) {
    // This method now provides look-ahead optimization for smoother movement

    // Check if we need to prepare for upcoming actions
    if (shouldJump(currentMove, nextMove)) {
        mControlState.jump = true;
    }

    // Check if we should start sprinting early for upcoming long movements
    if (shouldSprint(currentMove, nextMove)) {
        mControlState.sprint = true;
    }

    // Handle special movement cases
    if (nextMove.parkour) {
        // Prepare for parkour movement
        mControlState.sprint = true;
        mControlState.jump   = true;
    }

    // If the next move requires breaking blocks, slow down
    if (needsToBreakBlocks(nextMove)) {
        mControlState.sprint = false;
        mControlState.sneak  = true; // More precise movement
    }

    // If the next move requires placing blocks, also slow down
    if (needsToPlaceBlocks(nextMove)) {
        mControlState.sprint = false;
        mControlState.sneak  = true; // More precise movement
    }

    // Apply the updated control states
    applyControlStatesToPlayer();
}

bool BotPathfinder::shouldJump(Move const& from, Move const& to) const {
    // Jump if moving up or if it's a parkour move
    return to.y > from.y || to.parkour;
}

bool BotPathfinder::shouldSprint(Move const& from, Move const& to) const {
    // Sprint if movements allow it and we're moving horizontally
    if (!getMovements()->allowSprinting) return false;

    return std::abs(to.x - from.x) > 0 || std::abs(to.z - from.z) > 0;
}

bool BotPathfinder::needsToBreakBlocks(Move const& move) const {
    return !move.toBreak.empty();
}

bool BotPathfinder::needsToPlaceBlocks(Move const& move) const {
    return !move.toPlace.empty();
}

void BotPathfinder::clearControlStates() { mControlState = BotControlState{}; }

BotPathfinder::BreakBlockState BotPathfinder::breakBlock(BlockPos const& pos) {
    if (!mPlayer) return BreakBlockState::Fail;

    BlockSource* blockSource = getBlockSource();
    if (!blockSource) return BreakBlockState::Fail;

    Block const& block = blockSource->getBlock(pos);

    // Skip if block is already air
    if (mMovements->isEmptyBlock(block)) return BreakBlockState::Success;

    // Equip best tool for this block
    if (!equipBestTool(&block)) {
        // Continue anyway, might work with hands
    }

    // Move closer to the block if needed
    Vec3 blockCenter(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
    Vec3 currentPos = getCurrentPosition();

    if (mPlayer->simulateDestroyBlock(pos, ScriptModuleMinecraft::ScriptFacing::Up)) {
        // Too far to break, need to move closer first
        return BreakBlockState::Breaking;
    }
    return BreakBlockState::Fail;
}

bool BotPathfinder::placeBlock(BlockPos const& pos, Block const* block) {
    if (!mPlayer || !block) return false;

    BlockSource* blockSource = getBlockSource();
    if (!blockSource) return false;

    bool success =
        blockSource
            ->setBlock(pos, *block, BlockUpdateFlag::All, nullptr, nullptr, mPlayer);

    if (success) {
        // Simulate the placement animation/sound if needed
        // This could trigger events or effects
        return true;
    }

    return false;
}

bool BotPathfinder::equipBestTool(Block const* block) {
    if (!mPlayer || !block) return false;

    // This is a simplified tool selection logic
    // In a real implementation, you would:
    // 1. Get the player's inventory using mPlayer->getInventory()
    // 2. Analyze the block type and find the most efficient tool
    // 3. Switch to that tool slot using mPlayer->selectSlot(slotIndex)

    // For now, we'll implement a basic check based on block name/type
    std::string blockName = block->getTypeName();

    // Simple tool selection based on block name patterns
    if (blockName.find("stone") != std::string::npos
        || blockName.find("ore") != std::string::npos
        || blockName.find("cobblestone") != std::string::npos) {
        // Would select pickaxe if available
        // return selectToolByType("pickaxe");
    } else if (blockName.find("wood") != std::string::npos
               || blockName.find("log") != std::string::npos
               || blockName.find("plank") != std::string::npos) {
        // Would select axe if available
        // return selectToolByType("axe");
    } else if (blockName.find("dirt") != std::string::npos
               || blockName.find("grass") != std::string::npos
               || blockName.find("sand") != std::string::npos) {
        // Would select shovel if available
        // return selectToolByType("shovel");
    }

    // Return true for now, assuming we can break with current tool/hands
    return true;
}

Vec3 BotPathfinder::getCurrentPosition() const {
    if (mPlayer) {
        return mPlayer->getFeetPos();
    }
    return Vec3{0.0f, 0.0f, 0.0f};
}

BlockSource* BotPathfinder::getBlockSource() const {
    if (mPlayer) {
        return &mPlayer->getDimensionBlockSource();
    }
    return nullptr;
}

Dimension* BotPathfinder::getDimension() const {
    if (mPlayer) {
        return &mPlayer->getDimension();
    }
    return nullptr;
}

// These methods are now simple member setters (moved to header as inline)

// Event handlers
void BotPathfinder::onPathReset(std::string const& /* reason */) {
    // Handle path reset events
    mCurrentPathIndex = 0;
    clearControlStates();
    applyControlStatesToPlayer();
}

void BotPathfinder::onGoalUpdated(Goal const* /* goal */, bool /* dynamic */) {
    // Handle goal update events
    mCurrentPathIndex = 0;
}

void BotPathfinder::onPathUpdated(std::vector<std::unique_ptr<Move>> const& /* path */) {
    // Handle path update events
    mCurrentPathIndex = 0;
}

void BotPathfinder::applyControlStatesToPlayer() {
    if (!mPlayer) return;

    if (mControlState.sneak) {
        (void)mPlayer->simulateSneaking();
    } else {
        (void)mPlayer->simulateStopSneaking();
    }
    if (mControlState.jump) {
        (void)mPlayer->simulateJump();
    }
    if (mControlState.localDirection == 0.0f) {
        mPlayer->mSimulatedMovement->mType->emplace<0>();
    } else
        mPlayer->simulateMoveToLocation(
            mControlState.target,
            (1.0f + (mControlState.sprint ? 1.0f : 0.0f)) * 10000.0f,
            true
        );
}

void BotPathfinder::updateControlStateForPosition(Vec3 const& targetPos) {
    if (!mPlayer) return;

    Vec3   currentPos = getCurrentPosition();
    Vec3   direction  = targetPos - currentPos;
    double distance   = distanceHeuristic(currentPos, targetPos);

    if (distance < 0.1) {
        clearControlStates();
        applyControlStatesToPlayer();
        return;
    }

    // Normalize direction for movement
    direction = direction.normalize();

    // Clear previous control states
    clearControlStates();

    mControlState.localDirection = direction;
    mControlState.target         = targetPos;

    // Handle vertical movement
    if (direction.y > 0.5f || getCurrentPath()[mCurrentPathIndex]->parkour) {
        mControlState.jump = true;
    }
    const float threshold = 0.1f;

    // Enable sprint for longer distances
    if (distance > 3.0f
        && (std::abs(direction.x) > threshold || std::abs(direction.z) > threshold)) {
        mControlState.sprint = true;
    }

    // Apply the control states
    applyControlStatesToPlayer();
}

Block const* BotPathfinder::getScaffoldingBlock() const {
    // Return a default scaffolding block
    // In a real implementation, this could be configurable
    static optional_ref<Block const> scaffoldingBlock =
        Block::tryGetFromRegistry(HashedString("minecraft:stone"));
    return scaffoldingBlock ? &*scaffoldingBlock : nullptr;
}

std::vector<std::unique_ptr<Move>>
BotPathfinder::postProcessPath(std::vector<std::unique_ptr<Move>> path) const {
    if (path.empty() || !enablePathShortcut) {
        return path;
    }

    // Simple path optimization - remove intermediate points on straight lines
    std::vector<std::unique_ptr<Move>> optimizedPath;

    if (!path.empty()) {
        optimizedPath.push_back(std::make_unique<Move>(*path[0]));

        for (size_t i = 1; i < path.size() - 1; ++i) {
            Move const& prev = *path[i - 1];
            Move const& curr = *path[i];
            Move const& next = *path[i + 1];

            // Check if we can skip the current point
            bool canSkip =
                (curr.toBreak.empty() && curr.toPlace.empty()
                 && std::abs(next.y - prev.y) <= 1);

            if (!canSkip) {
                optimizedPath.push_back(std::make_unique<Move>(curr));
            }
        }

        if (path.size() > 1) {
            optimizedPath.push_back(std::make_unique<Move>(*path.back()));
        }
    }

    return optimizedPath;
}

void BotPathfinder::resetPath(std::string const& reason, bool clearStates) {
    mDebugPathGeo.reset();
    if (!mStopRequested && !mCurrentPath.empty() && mOnPathReset) {
        mOnPathReset(reason);
    }

    mCurrentPath.clear();
    mPathUpdated = false;

    // Reset path deviation tracking
    mLastProgressTick  = mCurrentTick;
    mLastProgressIndex = 0;

    if (clearStates) {
        mIsMoving   = false;
        mIsMining   = false;
        mIsBuilding = false;
    }

    if (mMovements) {
        mMovements->clearCollisionIndex();
    }

    if (mStopRequested) {
        mStopRequested = false;
    }
}
} // namespace we::bot
