#include "Movements.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "utils/BlockUtils.h"
#include <algorithm>
#include <cmath>

namespace we::bot {

// Cardinal directions: West, East, North, South
static constexpr std::array<std::pair<int, int>, 4> CARDINAL_DIRECTIONS = {
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}
};

// Diagonal directions
static constexpr std::array<std::pair<int, int>, 4> DIAGONAL_DIRECTIONS = {
    {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}
};

Movements::Movements(
    SimulatedPlayer*                     player,
    std::function<bool(BlockPos const&)> customBlockCheck
)
: mPlayer(player),
  mCustomBlockCheck(std::move(customBlockCheck)) {}

// Block checking functions (replacing unordered_set based checks)
bool Movements::canBreakBlockType(Block const& block) const {
    // Check basic properties
    if (isEmptyBlock(block)) return false;

    // Can't break bedrock, barrier blocks, etc.
    auto blockName = block.getTypeName();
    if (blockName == "minecraft:bedrock" || blockName == "minecraft:barrier"
        || blockName == "minecraft:structure_void") {
        return false;
    }

    return true;
}

bool Movements::shouldAvoidBlock(Block const& block) const {
    auto blockName = block.getTypeName();

    if (((uint64)block.getBlockType().mProperties & (uint64)BlockProperty::Unwalkable)
        != 0) {
        return true;
    }
    // Avoid dangerous blocks
    if (blockName == "minecraft:fire" || blockName == "minecraft:lava"
        || blockName == "minecraft:flowing_lava" || blockName == "minecraft:magma_block"
        || blockName == "minecraft:cactus" || blockName == "minecraft:sweet_berry_bush") {
        return true;
    }

    return false;
}

bool Movements::isEmptyBlock(Block const& block) const {
    // Use the built-in isAir() method
    return block.isAir();
}

bool Movements::isLiquidBlock(Block const& block) const {
    return ((uint64)block.getBlockType().mProperties & (uint64)BlockProperty::Liquid)
        != 0;
}

bool Movements::isPassableBlock(Block const& block) const {
    // Empty blocks are always passable
    if (isEmptyBlock(block)) return true;

    auto blockName = block.getTypeName();

    // List of blocks that can be walked through
    return blockName == "minecraft:grass" || blockName == "minecraft:tall_grass"
        || blockName == "minecraft:fern" || blockName == "minecraft:large_fern"
        || blockName == "minecraft:dead_bush" || blockName == "minecraft:torch"
        || blockName == "minecraft:redstone_torch" || blockName == "minecraft:lever"
        || blockName == "minecraft:button" || blockName == "minecraft:pressure_plate"
        || blockName == "minecraft:tripwire_hook" || blockName == "minecraft:tripwire"
        || blockName == "minecraft:vine" || blockName == "minecraft:ladder"
        || blockName == "minecraft:sign" || blockName == "minecraft:wall_sign";
}

bool Movements::isScaffoldingBlock(Block const& block) const {
    auto blockName = block.getTypeName();

    // Blocks that can be used for scaffolding
    return blockName == "minecraft:dirt" || blockName == "minecraft:cobblestone"
        || blockName == "minecraft:stone" || blockName == "minecraft:planks"
        || blockName == "minecraft:log" || blockName == "minecraft:scaffolding";
}

bool Movements::isInteractableBlock(Block const& block) const {
    auto blockName = block.getTypeName();

    // Blocks that can be interacted with (right-clicked)
    return blockName == "minecraft:door" || blockName == "minecraft:trapdoor"
        || blockName == "minecraft:fence_gate" || blockName == "minecraft:chest"
        || blockName == "minecraft:furnace" || blockName == "minecraft:crafting_table"
        || blockName == "minecraft:bed";
}
bool Movements::isClimbableBlock(Block const& block) const {
    return ((uint64)block.getBlockType().mProperties & (uint64)BlockProperty::Climbable)
        != 0;
}

// bool Movements::isPassableEntity(Block const& block) const {
//     // This function is for entities, not blocks
//     // For simplicity, we'll return false as entities are handled separately
//     return false;
// }

// bool Movements::shouldAvoidEntity(Block const& block) const {
//     // This function is for entities, not blocks
//     // For simplicity, we'll return false as entities are handled separately
//     return false;
// }

std::vector<std::unique_ptr<Move>> Movements::getNeighbors(Move const& move) {
    std::vector<std::unique_ptr<Move>> neighbors;

    // Simple moves in 4 cardinal directions - corresponds to JavaScript getNeighbors
    for (auto const& [dx, dz] : CARDINAL_DIRECTIONS) {
        getMoveForward(move, dx, dz, neighbors);
        getMoveJumpUp(move, dx, dz, neighbors);
        getMoveDropDown(move, dx, dz, neighbors);
        if (allowParkour) {
            getMoveParkourForward(move, dx, dz, neighbors);
        }
    }

    // Diagonal moves
    for (auto const& [dx, dz] : DIAGONAL_DIRECTIONS) {
        getMoveDiagonal(move, dx, dz, neighbors);
    }

    // Vertical moves
    getMoveDown(move, neighbors);
    getMoveUp(move, neighbors);

    return neighbors;
}

bool Movements::canWalkOn(BlockPos const& pos) const {
    Block const* block = getBlock(pos);
    if (!block) return false;

    // Can't walk on liquid
    if (isLiquidBlock(*block)) return false;

    // Can't walk on blocks we should avoid
    if (shouldAvoidBlock(*block)) return false;

    // Must be solid
    return !isEmptyBlock(*block) && !isPassableBlock(*block);
}

bool Movements::canWalkThrough(BlockPos const& pos) const {
    Block const* block = getBlock(pos);
    if (!block) return true; // Assume air if can't get block

    // Check if it's a passable block
    return isEmptyBlock(*block) || isPassableBlock(*block) || isLiquidBlock(*block);
}

bool Movements::isEmpty(BlockPos const& pos) const {
    Block const* block = getBlock(pos);
    if (!block) return true;

    return isEmptyBlock(*block);
}

bool Movements::isLiquid(BlockPos const& pos) const {
    Block const* block = getBlock(pos);
    if (!block) return false;

    return isLiquidBlock(*block);
}

bool Movements::isPassable(BlockPos const& pos) const {
    Block const* block = getBlock(pos);
    if (!block) return true;

    return isPassableBlock(*block);
}

Block const* Movements::getBlock(BlockPos const& pos) const {
    auto blockSource = getBlockSource();
    if (!blockSource) return nullptr;
    return &blockSource->getBlock(pos);
}

bool Movements::canBreakBlock(BlockPos const& pos) const {
    if (!canDig) return false;

    Block const* block = getBlock(pos);
    if (!block) return false;

    // Check if it's a block that can be broken
    if (!canBreakBlockType(*block)) return false;

    // Check exclusion areas
    if (isInExclusionArea(pos, exclusionAreasBreak)) return false;

    // Check if it would be safe to break
    return isSafeToBreak(pos);
}

bool Movements::canPlaceBlock(BlockPos const& pos) const {
    Block const* block = getBlock(pos);
    // if (!block || !block->getBlockType().mayPlace(*getBlockSource(), pos)) return
    // false;

    // Must be empty or passable
    // if (!isEmpty(pos) && !isPassable(pos)) return false;

    // Check exclusion areas
    if (isInExclusionArea(pos, exclusionAreasPlace)) return false;

    // Must have scaffolding items
    return hasScaffoldingItems();
}


float Movements::getBlockHeight(BlockPos const& pos) const {
    Block const* block = getBlock(pos);
    if (!block || block->isAir()) return pos.y;

    auto box =
        block->getBlockType().getCollisionShape(*block, *getBlockSource(), pos, nullptr);

    if (auto size = (box.max - box.min); size.x * size.y * size.z <= 0) {
        return pos.y;
    }
    return box.max.y;
}

size_t Movements::countScaffoldingItems() const {
    // size_t count = 0;
    // for(auto& item:  *mPlayer->mInventory->mInventory){
    //     if (item.isBlock() && item.mBlock && isScaffoldingBlock(*item.mBlock)) {
    //         count += item.mCount;
    //     }
    // }
    // return count;
    return 18 * 64;
}

bool Movements::hasScaffoldingItems() const { return countScaffoldingItems() > 0; }

bool Movements::isSafeToBreak(BlockPos const& pos) const {
    {
        Block const* below = getBlock({pos.x, pos.y - 1, pos.z});
        if (!below || isEmptyBlock(*below)) {
            // Can't break if there's no solid block below
            return false;
        }
    }
    if (dontMineUnderFallingBlock) {
        // Check if there are falling blocks above
        for (int y = pos.y + 1; y < pos.y + 2; ++y) {
            Block const* above = getBlock({pos.x, y, pos.z});
            if (!above) break;
            if (((uint64)above->getBlockType().mProperties
                 & (uint64)BlockProperty::CanFall)
                != 0) {
                return false;
            }
        }
    }
    return true;
}

bool Movements::isInExclusionArea(
    BlockPos const&                 pos,
    std::vector<BoundingBox> const& areas
) const {
    if (mCustomBlockCheck && mCustomBlockCheck(pos)) {
        return true;
    }
    for (auto const& area : areas) {
        if (area.contains(pos)) {
            return true;
        }
    }
    return false;
}

void Movements::updateCollisionIndex() {
    mEntityCollisions.clear();

    if (!allowEntityDetection) return;

    auto dimension = getDimension();
    if (!dimension) return;

    // This would need to be implemented with proper entity enumeration
    // For now, we'll leave it as a placeholder since we need access to
    // the dimension's entity list

    // TODO: Implement entity enumeration and collision detection
    // Example implementation would be:
    // auto entities = dimension->getAllEntities();
    // for (auto* entity : entities) {
    //     if (entity && shouldAvoidEntity(entity)) {
    //         Vec3 pos = entity->getPosition();
    //         Vec3 size = entity->getAABBDim();
    //         mEntityCollisions.push_back({pos, size, entity->getUniqueID()});
    //     }
    // }
}

void Movements::clearCollisionIndex() { mEntityCollisions.clear(); }

bool Movements::isEntityCollision(Vec3 const& pos) const {
    if (!allowEntityDetection) return false;

    for (auto const& collision : mEntityCollisions) {
        // Check if the position intersects with the entity's bounding box
        Vec3 entityMin = collision.position - collision.size * 0.5f;
        Vec3 entityMax = collision.position + collision.size * 0.5f;

        if (pos.x >= entityMin.x && pos.x <= entityMax.x && pos.y >= entityMin.y
            && pos.y <= entityMax.y && pos.z >= entityMin.z && pos.z <= entityMax.z) {
            return true;
        }
    }

    return false;
}

BlockSource* Movements::getBlockSource() const {
    if (mPlayer) {
        return &mPlayer->getDimensionBlockSource();
    }
    return nullptr;
}

Dimension* Movements::getDimension() const {
    if (mPlayer) {
        return &mPlayer->getDimension();
    }
    return nullptr;
}

// Movement generation methods corresponding to JavaScript implementation
void Movements::getMoveForward(
    Move const&                         node,
    int                                 dx,
    int                                 dz,
    std::vector<std::unique_ptr<Move>>& neighbors
) {
    BlockPos newPos(node.x + dx, node.y, node.z + dz);
    BlockPos blockB(node.x + dx, node.y + 1, node.z + dz); // Head space
    BlockPos blockC(node.x + dx, node.y, node.z + dz);     // Target position
    BlockPos blockD(node.x + dx, node.y - 1, node.z + dz); // Floor below target

    double                    cost = 1.0; // Base move cost
    std::vector<BlockToBreak> toBreak;
    std::vector<BlockToPlace> toPlace;

    // Check if we need to place a block below (scaffolding)
    if (!canWalkOn(blockD) && !isLiquid(blockC)) {
        if (node.remainingBlocks <= 0) return; // Not enough blocks

        Block const* blockDPtr = getBlock(blockD);
        if (!blockDPtr || !isEmpty(blockD)) {
            if (!canBreakBlock(blockD)) return;
            toBreak.emplace_back(blockD, blockDPtr);
        }

        if (!canPlaceBlock(blockD)) return;
        toPlace.emplace_back(blockD, nullptr); // Would need to specify block type
        cost += placeCost;
    }

    // Check if we can pass through head space
    Block const* blockBPtr = getBlock(blockB);
    if (blockBPtr && !isEmpty(blockB)) {
        if (!canBreakBlock(blockB)) return;
        toBreak.emplace_back(blockB, blockBPtr);
        cost += digCost;
    }

    // Check if we can pass through target position
    Block const* blockCPtr = getBlock(blockC);
    if (blockCPtr && !isEmpty(blockC)) {
        if (!canBreakBlock(blockC)) return;
        toBreak.emplace_back(blockC, blockCPtr);
        cost += digCost;
    }

    // Add liquid movement cost
    if (isLiquid({node.x, node.y, node.z})) {
        cost += liquidCost;
    }

    if (cost <= HUGE_COST) { // Cost threshold
        neighbors.push_back(
            std::make_unique<Move>(
                blockC.x,
                blockC.y,
                blockC.z,
                node.remainingBlocks - static_cast<int>(toPlace.size()),
                cost,
                toBreak,
                toPlace
            )
        );
    }
}

void Movements::getMoveJumpUp(
    Move const&                         node,
    int                                 dx,
    int                                 dz,
    std::vector<std::unique_ptr<Move>>& neighbors
) {
    BlockPos blockA(node.x, node.y + 2, node.z);           // Above current head
    BlockPos blockH(node.x + dx, node.y + 2, node.z + dz); // Above target head
    BlockPos blockB(node.x + dx, node.y + 1, node.z + dz); // Target head space
    BlockPos blockC(node.x + dx, node.y, node.z + dz);     // Target position

    double                    cost = 2.0; // Move + jump cost
    std::vector<BlockToBreak> toBreak;
    std::vector<BlockToPlace> toPlace;

    float heightBias = 0.0f;

    // Check if target position needs scaffolding
    if (!canWalkOn(blockC)) {
        if (node.remainingBlocks <= 0) return;

        BlockPos blockD(node.x + dx, node.y - 1, node.z + dz);
        if (!canWalkOn(blockD)) {
            if (node.remainingBlocks <= 1) return; // Need 2 blocks

            Block const* blockDPtr = getBlock(blockD);
            if (blockDPtr && !isEmpty(blockD) && !canBreakBlock(blockD)) return;
            if (blockDPtr && !isEmpty(blockD)) {
                toBreak.emplace_back(blockD, blockDPtr);
            }

            if (!canPlaceBlock(blockD)) return;
            toPlace.emplace_back(blockD, nullptr);
            cost += placeCost;
        }

        Block const* blockCPtr = getBlock(blockC);
        if (blockCPtr && !isEmpty(blockC) && !canBreakBlock(blockC)) return;
        if (blockCPtr && !isEmpty(blockC)) {
            toBreak.emplace_back(blockC, blockCPtr);
        }

        if (!canPlaceBlock(blockC)) return;
        toPlace.emplace_back(blockC, nullptr);
        cost       += placeCost;
        heightBias += 1.0f;
    }

    // Check jump height
    Block const* block0 = getBlock({node.x, node.y - 1, node.z});
    if (block0) {
        float  currentHeight = getBlockHeight({node.x, node.y - 1, node.z});
        float  targetHeight  = heightBias > 0 ? heightBias : getBlockHeight(blockC);
        double heightDiff    = targetHeight - currentHeight; // Jump height
        if (heightDiff > 1.2) return;                        // Too high to jump
    }

    // Check clearance above
    Block const* blockAPtr = getBlock(blockA);
    if (blockAPtr && !isEmpty(blockA)) {
        if (!canBreakBlock(blockA)) return;
        toBreak.emplace_back(blockA, blockAPtr);
        cost += digCost;
    }

    Block const* blockHPtr = getBlock(blockH);
    if (blockHPtr && !isEmpty(blockH)) {
        if (!canBreakBlock(blockH)) return;
        toBreak.emplace_back(blockH, blockHPtr);
        cost += digCost;
    }

    Block const* blockBPtr = getBlock(blockB);
    if (blockBPtr && !isEmpty(blockB)) {
        if (!canBreakBlock(blockB)) return;
        toBreak.emplace_back(blockB, blockBPtr);
        cost += digCost;
    }

    if (cost <= HUGE_COST) {
        neighbors.push_back(
            std::make_unique<Move>(
                blockB.x,
                blockB.y,
                blockB.z,
                node.remainingBlocks - static_cast<int>(toPlace.size()),
                cost,
                toBreak,
                toPlace
            )
        );
    }
}

void Movements::getMoveDropDown(
    Move const&                         node,
    int                                 dx,
    int                                 dz,
    std::vector<std::unique_ptr<Move>>& neighbors
) {
    BlockPos blockB(node.x + dx, node.y + 1, node.z + dz); // Head space above target
    BlockPos blockC(node.x + dx, node.y, node.z + dz);     // Target position
    BlockPos blockD(node.x + dx, node.y - 1, node.z + dz); // Below target

    double                    cost = 1.0;
    std::vector<BlockToBreak> toBreak;
    std::vector<BlockToPlace> toPlace;

    // Find landing block
    BlockPos landingPos;
    bool     foundLanding = false;
    int      maxDropDown  = 4;

    for (int dy = -2; dy >= -maxDropDown - 1; dy--) {
        BlockPos     checkPos(node.x + dx, node.y + dy, node.z + dz);
        Block const* block = getBlock(checkPos);

        if (!block) break;

        if (isLiquid(checkPos) && canWalkThrough(checkPos)) {
            landingPos   = {checkPos.x, checkPos.y + 1, checkPos.z};
            foundLanding = true;
            break;
        }

        if (canWalkOn(checkPos)) {
            landingPos   = {checkPos.x, checkPos.y + 1, checkPos.z};
            foundLanding = true;
            break;
        }

        if (!canWalkThrough(checkPos)) break;
    }

    if (!foundLanding) return;
    if ((node.y - landingPos.y) > maxDropDown) return;

    // Check clearance during fall
    Block const* blockBPtr = getBlock(blockB);
    if (blockBPtr && !isEmpty(blockB)) {
        if (!canBreakBlock(blockB)) return;
        toBreak.emplace_back(blockB, blockBPtr);
        cost += digCost;
    }

    Block const* blockCPtr = getBlock(blockC);
    if (blockCPtr && !isEmpty(blockC)) {
        if (!canBreakBlock(blockC)) return;
        toBreak.emplace_back(blockC, blockCPtr);
        cost += digCost;
    }

    Block const* blockDPtr = getBlock(blockD);
    if (blockDPtr && !isEmpty(blockD)) {
        if (!canBreakBlock(blockD)) return;
        toBreak.emplace_back(blockD, blockDPtr);
        cost += digCost;
    }

    // Don't go underwater
    if (isLiquid(blockC)) return;

    if (cost <= HUGE_COST) {
        neighbors.push_back(
            std::make_unique<Move>(
                landingPos.x,
                landingPos.y,
                landingPos.z,
                node.remainingBlocks - static_cast<int>(toPlace.size()),
                cost,
                toBreak,
                toPlace
            )
        );
    }
}

void Movements::getMoveParkourForward(
    Move const&                         node,
    int                                 dx,
    int                                 dz,
    std::vector<std::unique_ptr<Move>>& neighbors
) {
    Block const* block0 = getBlock({node.x, node.y - 1, node.z});
    Block const* block1 = getBlock({node.x + dx, node.y - 1, node.z + dz});

    // Check parkour preconditions
    if (!block0 || !canWalkOn({node.x, node.y - 1, node.z})) return;
    if (block1 && canWalkOn({node.x + dx, node.y - 1, node.z + dz})) return;
    if (!canWalkThrough({node.x + dx, node.y, node.z + dz})) return;
    if (!canWalkThrough({node.x + dx, node.y + 1, node.z + dz})) return;
    if (isLiquid({node.x, node.y, node.z})) return; // Can't jump from water

    double cost        = 1.0;
    int    maxDistance = allowSprinting ? 4 : 2;

    // Check ceiling clearance
    bool ceilingClear = canWalkThrough({node.x, node.y + 2, node.z})
                     && canWalkThrough({node.x + dx, node.y + 2, node.z + dz});

    // Check floor clearance for downward jumps
    bool floorCleared = !canWalkOn({node.x + dx, node.y - 2, node.z + dz});

    for (int d = 2; d <= maxDistance; d++) {
        int targetX = node.x + dx * d;
        int targetZ = node.z + dz * d;

        BlockPos blockA(targetX, node.y + 2, targetZ); // Above head at target
        BlockPos blockB(targetX, node.y + 1, targetZ); // Head space at target
        BlockPos blockC(targetX, node.y, targetZ);     // Target position
        BlockPos blockD(targetX, node.y - 1, targetZ); // Below target

        if (!canWalkThrough(blockC)) cost += entityCost;

        // Forward parkour (same level)
        if (ceilingClear && canWalkThrough(blockB) && canWalkThrough(blockC)
            && canWalkOn(blockD)) {
            neighbors.push_back(
                std::make_unique<Move>(
                    blockC.x,
                    blockC.y,
                    blockC.z,
                    node.remainingBlocks,
                    cost,
                    std::vector<BlockToBreak>(),
                    std::vector<BlockToPlace>(),
                    true // parkour flag
                )
            );
            break;
        }
        // Upward parkour (jump up 1 block)
        else if (ceilingClear && canWalkThrough(blockB) && canWalkOn(blockC) && d != 4) {
            if (canWalkThrough(blockA)) {
                float  currentHeight = getBlockHeight({node.x, node.y - 1, node.z});
                float  targetHeight  = getBlockHeight(blockC);
                double heightDiff    = targetHeight - currentHeight;
                if (heightDiff <= 1.2) { // Not too high to jump
                    neighbors.push_back(
                        std::make_unique<Move>(
                            blockB.x,
                            blockB.y,
                            blockB.z,
                            node.remainingBlocks,
                            cost,
                            std::vector<BlockToBreak>(),
                            std::vector<BlockToPlace>(),
                            true
                        )
                    );
                    break;
                }
            }
        }
        // Downward parkour (fall down)
        else if ((ceilingClear || d == 2) && canWalkThrough(blockB)
                 && canWalkThrough(blockC) && canWalkThrough(blockD) && floorCleared) {
            BlockPos blockE(targetX, node.y - 2, targetZ);
            if (canWalkOn(blockE)) {
                neighbors.push_back(
                    std::make_unique<Move>(
                        blockD.x,
                        blockD.y,
                        blockD.z,
                        node.remainingBlocks,
                        cost,
                        std::vector<BlockToBreak>(),
                        std::vector<BlockToPlace>(),
                        true
                    )
                );
            }
            floorCleared = floorCleared && !canWalkOn(blockE);
        } else if (!canWalkThrough(blockB) || !canWalkThrough(blockC)) {
            break;
        }

        ceilingClear = ceilingClear && canWalkThrough(blockA);
    }
}

// Helper method to find landing block for drop down movements
BlockPos Movements::getLandingBlock(Move const& node, int dx, int dz) const {
    BlockPos checkPos = {node.x + dx, node.y - 2, node.z + dz};
    int      minY     = -64; // Default Minecraft minimum Y level

    while (checkPos.y > minY) {
        Block const* block = getBlock(checkPos);
        if (!block) break;

        // If we find a liquid block that's safe, land on it
        if (isLiquid(checkPos) && canWalkThrough(checkPos)) {
            return checkPos;
        }

        // If we find a solid block, check if it's within drop distance
        if (canWalkOn(checkPos)) {
            int dropDistance = node.y - checkPos.y;
            if (dropDistance <= 10) { // maxDropDown equivalent
                return {
                    checkPos.x,
                    checkPos.y + 1,
                    checkPos.z
                }; // Land on top of the block
            }
            return {-1, -1, -1}; // Invalid position - too high
        }

        // If the block is not safe, stop searching
        if (!canWalkThrough(checkPos)) {
            return {-1, -1, -1}; // Invalid position
        }

        checkPos.y--;
    }

    return {-1, -1, -1}; // Invalid position
}

void Movements::getMoveDown(
    Move const&                         node,
    std::vector<std::unique_ptr<Move>>& neighbors
) {
    double                    cost = 1.0; // move cost
    std::vector<BlockToBreak> toBreak;
    std::vector<BlockToPlace> toPlace;

    BlockPos landingPos = getLandingBlock(node, 0, 0);
    if (landingPos.x == -1) return; // No valid landing block

    // Check if we need to break the block below
    BlockPos belowPos = {node.x, node.y - 1, node.z};
    if (!canWalkThrough(belowPos)) {
        if (!canBreakBlock(belowPos)) return;
        toBreak.push_back({belowPos, 0});
        cost += digCost;
    }

    if (cost > HUGE_COST) return;

    // Don't go underwater
    if (isLiquid({node.x, node.y, node.z})) return;

    // Add entity cost if needed
    cost += entityCost; // Simplified entity cost

    neighbors.push_back(
        std::make_unique<Move>(
            landingPos.x,
            landingPos.y,
            landingPos.z,
            node.remainingBlocks - static_cast<int>(toPlace.size()),
            cost,
            toBreak,
            toPlace
        )
    );
}

void Movements::getMoveUp(
    Move const&                         node,
    std::vector<std::unique_ptr<Move>>& neighbors
) {
    if (isLiquid({node.x, node.y, node.z})) return;

    double                    cost = 1.0; // move cost
    std::vector<BlockToBreak> toBreak;
    std::vector<BlockToPlace> toPlace;

    // Check if we need to break blocks above
    BlockPos abovePos = {node.x, node.y + 2, node.z};
    if (!canWalkThrough(abovePos)) {
        if (!canBreakBlock(abovePos)) return;
        toBreak.push_back({abovePos, 0});
        cost += digCost;
    }

    if (cost > HUGE_COST) return;

    // Check if we need to place a block to climb
    BlockPos currentPos  = {node.x, node.y, node.z};
    bool     isClimbable = false;
    {
        Block const* block = getBlock(currentPos);
        if (block && isClimbableBlock(*block)) {
            isClimbable = true;
        }
    }

    if (!isClimbable) {
        if (!allow1by1towers || node.remainingBlocks == 0) return;

        if (!isEmpty(currentPos)) {
            if (!canBreakBlock(currentPos)) return;
            toBreak.push_back({currentPos, 0});
        }

        BlockPos belowPos = {node.x, node.y - 1, node.z};
        if (canWalkOn(belowPos)) {
            // Check height difference for jump-place
            float  belowHeight   = getBlockHeight(belowPos);
            float  currentHeight = getBlockHeight(currentPos); // Player position
            double heightDiff    = belowHeight - currentHeight;
            if (heightDiff < -0.2) return; // Cannot jump-place from too low
        }

        if (!canPlaceBlock(currentPos)) return;
        toPlace.push_back(BlockToPlace(currentPos, nullptr)); // jump place
        cost += placeCost;
    }

    if (cost > HUGE_COST) return;

    BlockPos targetPos = {node.x, node.y + 1, node.z};
    neighbors.push_back(
        std::make_unique<Move>(
            targetPos.x,
            targetPos.y,
            targetPos.z,
            node.remainingBlocks - static_cast<int>(toPlace.size()),
            cost,
            toBreak,
            toPlace
        )
    );
}

void Movements::getMoveDiagonal(
    Move const&                         node,
    int                                 dx,
    int                                 dz,
    std::vector<std::unique_ptr<Move>>& neighbors
) {
    double                    cost = 1.4142135623730951; // sqrt(2) - diagonal move cost
    std::vector<BlockToBreak> toBreak;
    std::vector<BlockToPlace> toPlace;

    BlockPos targetPos = {node.x + dx, node.y, node.z + dz};
    int      y         = canWalkOn(targetPos)
                           ? 1
                           : 0; // Landing block or standing on block when jumping up by 1

    BlockPos belowCurrent = {node.x, node.y - 1, node.z};

    // Calculate cost for path 1 (through Z direction first)
    double                    cost1 = 0;
    std::vector<BlockToBreak> toBreak1;

    BlockPos path1Blocks[] = {
        {node.x, node.y + y + 1, node.z + dz}, // B1
        {node.x, node.y + y,     node.z + dz}, // C1
        {node.x, node.y + y - 1, node.z + dz}  // D1
    };

    for (auto& pos : path1Blocks) {
        if (!canWalkThrough(pos)) {
            if (!canBreakBlock(pos)) {
                cost1 += HUGE_COST;
                break;
            }
            toBreak1.push_back({pos, 0});
            cost1 += digCost;
        }
    }

    // Calculate cost for path 2 (through X direction first)
    double                    cost2 = 0;
    std::vector<BlockToBreak> toBreak2;

    BlockPos path2Blocks[] = {
        {node.x + dx, node.y + y + 1, node.z}, // B2
        {node.x + dx, node.y + y,     node.z}, // C2
        {node.x + dx, node.y + y - 1, node.z}  // D2
    };

    for (auto& pos : path2Blocks) {
        if (!canWalkThrough(pos)) {
            if (!canBreakBlock(pos)) {
                cost2 += HUGE_COST;
                break;
            }
            toBreak2.push_back({pos, 0});
            cost2 += digCost;
        }
    }

    // Choose the cheaper path
    if (cost1 < cost2) {
        cost += cost1;
        toBreak.insert(toBreak.end(), toBreak1.begin(), toBreak1.end());
    } else {
        cost += cost2;
        toBreak.insert(toBreak.end(), toBreak2.begin(), toBreak2.end());
    }

    if (cost > HUGE_COST) return;

    // Check destination blocks
    BlockPos destBlocks[] = {
        {node.x + dx, node.y + y,     node.z + dz}, // destination
        {node.x + dx, node.y + y + 1, node.z + dz}  // above destination
    };

    for (auto& pos : destBlocks) {
        if (!canWalkThrough(pos)) {
            if (!canBreakBlock(pos)) return;
            toBreak.push_back({pos, 0});
            cost += digCost;
        }
    }

    if (cost > HUGE_COST) return;

    // Add liquid cost if moving from liquid
    if (isLiquid({node.x, node.y, node.z})) cost += liquidCost;

    // Determine final target position
    BlockPos finalTarget;
    BlockPos belowTarget = {node.x + dx, node.y - 1, node.z + dz};

    if (y == 1) { // Case jump up by 1
        float  currentHeight = getBlockHeight(belowCurrent);
        float  targetHeight  = getBlockHeight(targetPos);
        double heightDiff    = targetHeight - currentHeight;
        if (heightDiff > 1.2) return; // Too high to jump

        BlockPos aboveCurrent = {node.x, node.y + 2, node.z};
        if (!canWalkThrough(aboveCurrent)) {
            if (!canBreakBlock(aboveCurrent)) return;
            toBreak.push_back({aboveCurrent, 0});
            cost += digCost;
        }

        if (cost > HUGE_COST) return;
        cost        += 1; // jump cost
        finalTarget  = {node.x + dx, node.y + 1, node.z + dz};
    } else if (canWalkOn(belowTarget) || isLiquid(targetPos)) {
        finalTarget = targetPos;
    } else {
        BlockPos belowBelowTarget = {node.x + dx, node.y - 2, node.z + dz};
        if (canWalkOn(belowBelowTarget) || isLiquid(belowTarget)) {
            if (!canWalkThrough(belowTarget)) return; // don't walk into unsafe blocks
            cost        += entityCost;                // simplified entity cost
            finalTarget  = belowTarget;
        } else {
            return; // No valid target
        }
    }

    neighbors.push_back(
        std::make_unique<Move>(
            finalTarget.x,
            finalTarget.y,
            finalTarget.z,
            node.remainingBlocks - static_cast<int>(toPlace.size()),
            cost,
            toBreak,
            toPlace
        )
    );
}

} // namespace we::bot