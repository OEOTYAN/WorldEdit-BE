#include "SimulatedPlayerBuilder.h"
#include "bot/BotPathfinder.h"
#include "bot/pathfinder/Goals.h"
#include "data/LocalContext.h"
#include "worldedit/WorldEdit.h"

namespace we {

SimulatedPlayerBuilder::SimulatedPlayerBuilder(LocalContext& context, size_t maxPlayers)
: Builder(context),
  mMaxPlayers(maxPlayers),
  mPlayerIdleTimeout(std::chrono::seconds(30)),
  mTasksPerTick(1),
  mTasksCompleted(0),
  mTasksFailed(0),
  mActivePlayers(0) {}

void SimulatedPlayerBuilder::setup() {
    mIsRunning = true;

    // Launch main processing coroutine
    ll::coro::keepThis(
        [self = static_pointer_cast<SimulatedPlayerBuilder>(shared_from_this())]()
            -> ll::coro::CoroTask<> {
            while (self->mIsRunning) {
                co_await self->processTasksAsync();
                co_await std::chrono::milliseconds(50);
            }
        }
    ).launch(ll::thread::ServerThreadExecutor::getDefault());

    type = BuilderType::Bot;

    // Reserve space for player pool
    mPlayerPool.reserve(mMaxPlayers);
}
void SimulatedPlayerBuilder::remove() {
    WE_DEBUG("SimulatedPlayerBuilder::remove called");
    mIsRunning = false;
    // Clean up all players and tasks
    clearTasks();
    removeAllPlayers();
}

SimulatedPlayerBuilder::~SimulatedPlayerBuilder() { remove(); }

// ManagedSimulatedPlayer implementation
ManagedSimulatedPlayer::ManagedSimulatedPlayer(
    SimulatedPlayer*                     player,
    DimensionType                        dimension,
    std::function<bool(BlockPos const&)> customBlockCheck
)
: player(player),
  dimension(dimension),
  isIdle(true),
  lastActivityTime(std::chrono::steady_clock::now()) {
    // Create individual pathfinder for this player
    pathfinder =
        std::make_unique<bot::BotPathfinder>(player, std::move(customBlockCheck));
}

bool SimulatedPlayerBuilder::setBlock(
    BlockSource&                blockSource,
    BlockPos const&             pos,
    Block const&                block,
    std::shared_ptr<BlockActor> blockActor
) const {
    // Queue the task instead of immediate execution
    return const_cast<SimulatedPlayerBuilder*>(this)->setBlockQueued(
        blockSource,
        pos,
        block,
        blockActor,
        TaskPriority::Normal,
        nullptr
    );
}

bool SimulatedPlayerBuilder::setBlockQueued(
    BlockSource&                blockSource,
    BlockPos const&             pos,
    Block const&                block,
    std::shared_ptr<BlockActor> blockActor,
    TaskPriority                priority,
    std::function<void(bool)>   callback
) {
    // Get dimension from BlockSource
    DimensionType dimension = getDimensionFromBlockSource(blockSource);

    mPendingPositions.insert(pos);

    // Create new task with dimension info
    auto task = std::make_shared<BlockTask>(
        pos,
        &block,
        dimension,
        blockActor,
        priority,
        callback
    );
    addTask(task);
    return true;
}

void SimulatedPlayerBuilder::addTask(std::shared_ptr<BlockTask> task) {
    if (!task) return;
    mTaskQueue.push(task);
}

void SimulatedPlayerBuilder::clearTasks() {
    // Clear the queue
    while (!mTaskQueue.empty()) {
        auto task = mTaskQueue.top();
        mTaskQueue.pop();

        task->status = TaskStatus::Cancelled;
        if (task->callback) {
            task->callback(false);
        }
    }

    // Clear current tasks from players
    for (auto& managedPlayer : mPlayerPool) {
        if (managedPlayer->currentTask) {
            managedPlayer->currentTask->status = TaskStatus::Cancelled;
            if (managedPlayer->currentTask->callback) {
                managedPlayer->currentTask->callback(false);
            }
            managedPlayer->currentTask.reset();
            managedPlayer->isIdle = true;
        }
    }
}

// New per-player methods
bot::BotPathfinder* SimulatedPlayerBuilder::getPlayerPathfinder(SimulatedPlayer* player) {
    for (auto& managedPlayer : mPlayerPool) {
        if (managedPlayer->player == player) {
            return managedPlayer->pathfinder.get();
        }
    }
    return nullptr;
}

void SimulatedPlayerBuilder::stopPlayerPathfinding(SimulatedPlayer* player) {
    auto pathfinder = getPlayerPathfinder(player);
    if (pathfinder) {
        pathfinder->stop();
    }
}

bool SimulatedPlayerBuilder::isPlayerPathfinding(SimulatedPlayer* player) const {
    for (const auto& managedPlayer : mPlayerPool) {
        if (managedPlayer->player == player && managedPlayer->pathfinder) {
            return managedPlayer->pathfinder->isMoving();
        }
    }
    return false;
}

Vec3 SimulatedPlayerBuilder::getPlayerCurrentPosition(SimulatedPlayer* player) const {
    for (const auto& managedPlayer : mPlayerPool) {
        if (managedPlayer->player == player && managedPlayer->pathfinder) {
            return managedPlayer->pathfinder->getCurrentPosition();
        }
    }
    return Vec3{0.0f, 0.0f, 0.0f};
}

bool SimulatedPlayerBuilder::createSimulatedPlayer(
    const std::string& name,
    DimensionType      dimension
) {
    if (mPlayerPool.size() >= mMaxPlayers) {
        return false;
    }

    // Create SimulatedPlayer in the specified dimension
    auto* player = createSimulatedPlayerInDimension(name, dimension);
    if (!player) {
        return false;
    }

    auto managedPlayer = std::make_unique<ManagedSimulatedPlayer>(
        player,
        dimension,
        [this](BlockPos const& pos) { return mPendingPositions.contains(pos); }
    );
    mPlayerPool.push_back(std::move(managedPlayer));
    mActivePlayers++;

    return true;
}

SimulatedPlayer* SimulatedPlayerBuilder::createSimulatedPlayerInDimension(
    const std::string& name,
    DimensionType      dimension
) {
    // Create SimulatedPlayer using LeviLamina API
    // The player will be created in the server's spawn dimension first
    auto* player = context.createSimulatedPlayer(name);
    if (!player) {
        return nullptr;
    }

    // If the target dimension is different from spawn, teleport the player
    if (dimension != player->getDimensionId()) {
        // TODO: Implement dimension teleportation using proper LeviLamina API
        // This requires access to the level and dimension management APIs
        // For now, the player will be created in the default dimension
    }

    return player;
}

void SimulatedPlayerBuilder::removeIdlePlayers() {
    auto now = std::chrono::steady_clock::now();

    for (int i = static_cast<int>(mPlayerPool.size()) - 1; i >= 0; --i) {
        auto& managedPlayer = mPlayerPool[i];

        if (managedPlayer->isIdle
            && std::chrono::duration_cast<std::chrono::milliseconds>(
                   now - managedPlayer->lastActivityTime
               ) > mPlayerIdleTimeout) {
            removePlayer(i);
        }
    }
}

void SimulatedPlayerBuilder::removeAllPlayers() {
    while (!mPlayerPool.empty()) {
        removePlayer(mPlayerPool.size() - 1);
    }
}


void SimulatedPlayerBuilder::removePlayer(size_t index) {
    if (index >= mPlayerPool.size()) return;

    auto& managedPlayer = mPlayerPool[index];

    // Cancel current task if any
    if (managedPlayer->currentTask) {
        managedPlayer->currentTask->status = TaskStatus::Cancelled;
        if (managedPlayer->currentTask->callback) {
            managedPlayer->currentTask->callback(false);
        }
    }
    if (managedPlayer->player) {
        managedPlayer->player->kill();
        managedPlayer->player->disconnect();
        managedPlayer->player->remove();
        managedPlayer->player->setGameTestHelper(nullptr);
    }
    // Remove the player
    mPlayerPool.erase(mPlayerPool.begin() + index);
    mActivePlayers--;
}

ManagedSimulatedPlayer* SimulatedPlayerBuilder::findIdlePlayer(DimensionType dimension) {
    for (auto& managedPlayer : mPlayerPool) {
        if (managedPlayer->isIdle && managedPlayer->dimension == dimension) {
            return managedPlayer.get();
        }
    }
    return nullptr;
}

ll::coro::CoroTask<> SimulatedPlayerBuilder::processTasksAsync() {
    // Process tasks
    size_t tasksProcessed = 0;
    while (true) {
        bool hasDeadPlayers = false;
        for (auto& managedPlayer : mPlayerPool) {
            if (!managedPlayer->player->isAlive() || managedPlayer->player->mRemoved) {
                handleTaskFailure(managedPlayer->currentTask, managedPlayer.get());
                removePlayer(&managedPlayer - &mPlayerPool[0]);
                hasDeadPlayers = true;
                break;
            }
        }
        if (!hasDeadPlayers) break;
    }
    // Check completion of current tasks
    for (auto& managedPlayer : mPlayerPool) {
        if (!managedPlayer->pathfinder->tick()) {
            assignTaskToPlayerAsync(managedPlayer->currentTask, managedPlayer.get());
        }
        if (!managedPlayer->isIdle && managedPlayer->currentTask) {
            co_await processTaskAsync(managedPlayer.get());
        }
    }

    // Assign new tasks to idle players
    while (!mTaskQueue.empty() && tasksProcessed < mTasksPerTick) {
        auto task = mTaskQueue.top();

        // Find an idle player in the same dimension as the task
        auto* idlePlayer = findIdlePlayer(task->dimension);
        if (!idlePlayer) {
            // Try to create a new player in the task's dimension if we have capacity
            if (mPlayerPool.size() < mMaxPlayers
                && createSimulatedPlayer(
                    fmt::format(
                        "BotBuilder_{}_{}",
                        context.getUuid().asString().substr(0, 4),
                        mPlayerPool.size()
                    ),
                    task->dimension
                )) {
                idlePlayer = findIdlePlayer(task->dimension);
            }
        }

        if (!idlePlayer) break; // No available player for this dimension

        mTaskQueue.pop();
        co_await assignTaskToPlayerAsync(task, idlePlayer);
        tasksProcessed++;
    }

    // Clean up idle players periodically
    static int cleanupCounter = 0;
    if (++cleanupCounter >= 100) { // Every ~5 seconds at 20 TPS
        cleanupCounter = 0;
        removeIdlePlayers();
    }
}

ll::coro::CoroTask<> SimulatedPlayerBuilder::assignTaskToPlayerAsync(
    std::shared_ptr<BlockTask> task,
    ManagedSimulatedPlayer*    managedPlayer
) {
    if (!task || !managedPlayer || !managedPlayer->player) {
        co_return;
    }

    managedPlayer->currentTask      = task;
    managedPlayer->isIdle           = false;
    managedPlayer->lastActivityTime = std::chrono::steady_clock::now();

    task->status    = TaskStatus::InProgress;
    task->startTime = std::chrono::steady_clock::now();

    // Use pathfinding to navigate to the target position using player's own pathfinder
    auto goal =
        std::make_unique<bot::GoalNear>(task->pos.x, task->pos.y + 1, task->pos.z, 2.0);

    // Use async pathfinding
    auto result = co_await managedPlayer->pathfinder->getPathToAsync(*goal);
    if (result.status != bot::ComputedPath::Status::Success)
        managedPlayer->player->simulateChat(
            fmt::format("{} {}", result.status, result.path.size())
        );
    if (!result.path.empty()) {
        // Set the path for execution
        managedPlayer->pathfinder->setExecutingPath(std::move(result.path));
    } else {
        // Navigation failed, mark task as failed
        if (managedPlayer->currentTask) {
            handleTaskFailure(managedPlayer->currentTask, managedPlayer);
        }
    }
}

ll::coro::CoroTask<>
SimulatedPlayerBuilder::processTaskAsync(ManagedSimulatedPlayer* managedPlayer) {
    if (!managedPlayer || !managedPlayer->currentTask) {
        co_return;
    }

    auto& task = managedPlayer->currentTask;

    // Check if player reached the target position
    Vec3 currentPos = managedPlayer->pathfinder->getCurrentPosition();
    Vec3 targetPos(
        static_cast<float>(task->pos.x) + 0.5f,
        static_cast<float>(task->pos.y) + 1.0f,
        static_cast<float>(task->pos.z) + 0.5f
    );

    double distance = (currentPos - targetPos).length();

    if (distance > 3.0) {
        if (!managedPlayer->pathfinder->isExecuting()) {
            handleTaskFailure(task, managedPlayer);
        }
        co_return; // Wait for player to reach position
    }

    // Use SimulatedPlayer to place the block
    auto* blockSource = &managedPlayer->player->getDimensionBlockSource();

    // Check if the target position matches the task dimension
    if (blockSource->getDimensionId() != static_cast<int>(task->dimension)) {
        // Dimension mismatch, fail the task
        handleTaskFailure(task, managedPlayer);
        co_return;
    }

    // Use the simulated player to interact/place block
    bool success = false;

    success = managedPlayer->pathfinder->placeBlock(task->pos, task->block);

    if (success) {
        task->status = TaskStatus::Completed;
        mTasksCompleted++;

        if (task->callback) {
            task->callback(true);
        }
    } else {
        handleTaskFailure(task, managedPlayer);
        co_return;
    }

    // Reset to idle
    managedPlayer->currentTask.reset();
    managedPlayer->isIdle           = true;
    managedPlayer->lastActivityTime = std::chrono::steady_clock::now();
}

void SimulatedPlayerBuilder::handleTaskFailure(
    std::shared_ptr<BlockTask> task,
    ManagedSimulatedPlayer*    managedPlayer
) {
    if (!task || !managedPlayer) return;

    task->retryCount++;

    if (task->retryCount < BlockTask::MAX_RETRIES) {
        // Retry the task
        task->priority =
            (TaskPriority)((int)task->priority + 1); // Increase priority on retry
        task->status      = TaskStatus::Pending;
        task->createdTime = std::chrono::steady_clock::now();
        mTaskQueue.push(task);
    } else {
        // Mark as failed
        task->status = TaskStatus::Failed;
        mTasksFailed++;

        if (task->callback) {
            task->callback(false);
        }
    }

    // Reset player to idle
    managedPlayer->currentTask.reset();
    managedPlayer->isIdle           = true;
    managedPlayer->lastActivityTime = std::chrono::steady_clock::now();
}

void SimulatedPlayerBuilder::cleanupCompletedTasks() {
    // This method can be called periodically to clean up any remaining references
    // to completed tasks, but most cleanup is handled inline
}

DimensionType
SimulatedPlayerBuilder::getDimensionFromBlockSource(BlockSource& blockSource) const {
    return blockSource.getDimensionId();
}

} // namespace we