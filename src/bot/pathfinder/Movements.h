#pragma once

#include "Move.h"
#include "worldedit/Global.h"
#include <memory>
#include <vector>

namespace we::bot {

class Movements {
public:
    // Movement costs
    double digCost    = 1.0;
    double placeCost  = 1.0;
    double liquidCost = 1.0;
    double entityCost = 1.0;

    static constexpr double HUGE_COST = 10000.0;

    // Movement flags
    bool canDig                    = true;
    bool dontCreateFlow            = true;
    bool dontMineUnderFallingBlock = true;
    bool allow1by1towers           = true;
    bool allowFreeMotion           = false;
    bool allowParkour              = false;
    bool allowSprinting            = true;
    bool allowEntityDetection      = true;

    // Block checking functions (replacing block sets)
    bool canBreakBlockType(Block const& block) const;
    bool shouldAvoidBlock(Block const& block) const;
    bool isEmptyBlock(Block const& block) const;
    bool isLiquidBlock(Block const& block) const;
    bool isPassableBlock(Block const& block) const;
    bool isScaffoldingBlock(Block const& block) const;
    bool isInteractableBlock(Block const& block) const;
    bool isClimbableBlock(Block const& block) const;
    // bool isPassableEntity(Block const& block) const;
    // bool shouldAvoidEntity(Block const& block) const;

    // Exclusion areas
    std::vector<BoundingBox> exclusionAreasStep;
    std::vector<BoundingBox> exclusionAreasBreak;
    std::vector<BoundingBox> exclusionAreasPlace;

private:
    SimulatedPlayer* mPlayer;

    // Entity collision tracking
    struct EntityCollision {
        Vec3 position;
        Vec3 size;
        int  entityId;
    };
    std::vector<EntityCollision> mEntityCollisions;

    std::function<bool(BlockPos const&)> mCustomBlockCheck;

public:
    explicit Movements(
        SimulatedPlayer*                     player,
        std::function<bool(BlockPos const&)> customBlockCheck
    );

    // Core pathfinding method
    std::vector<std::unique_ptr<Move>> getNeighbors(Move const& move);

    // Movement validation
    bool canWalkOn(BlockPos const& pos) const;
    bool canWalkThrough(BlockPos const& pos) const;
    bool canPlaceBlock(BlockPos const& pos) const;
    bool canBreakBlock(BlockPos const& pos) const;
    bool isLiquid(BlockPos const& pos) const;
    bool isEmpty(BlockPos const& pos) const;
    bool isPassable(BlockPos const& pos) const;

    float getBlockHeight(BlockPos const& pos) const;

    // Scaffolding methods
    size_t countScaffoldingItems() const;
    bool   hasScaffoldingItems() const;

    // Entity collision methods
    void updateCollisionIndex();
    void clearCollisionIndex();
    bool isEntityCollision(Vec3 const& pos) const;

    // Exclusion area methods
    bool
    isInExclusionArea(BlockPos const& pos, std::vector<BoundingBox> const& areas) const;

    // Block utility methods
    Block const* getBlock(BlockPos const& pos) const;
    bool         isBlockType(BlockPos const& pos, int blockId) const;

    // Player access methods
    BlockSource*     getBlockSource() const;
    Dimension*       getDimension() const;
    SimulatedPlayer* getPlayer() const { return mPlayer; }

    // Movement generation methods corresponding to JavaScript implementation
    void getMoveForward(
        Move const&                         node,
        int                                 dx,
        int                                 dz,
        std::vector<std::unique_ptr<Move>>& neighbors
    );
    void getMoveJumpUp(
        Move const&                         node,
        int                                 dx,
        int                                 dz,
        std::vector<std::unique_ptr<Move>>& neighbors
    );
    void getMoveDropDown(
        Move const&                         node,
        int                                 dx,
        int                                 dz,
        std::vector<std::unique_ptr<Move>>& neighbors
    );
    void getMoveParkourForward(
        Move const&                         node,
        int                                 dx,
        int                                 dz,
        std::vector<std::unique_ptr<Move>>& neighbors
    );
    void getMoveDown(Move const& node, std::vector<std::unique_ptr<Move>>& neighbors);
    void getMoveUp(Move const& node, std::vector<std::unique_ptr<Move>>& neighbors);
    void getMoveDiagonal(
        Move const&                         node,
        int                                 dx,
        int                                 dz,
        std::vector<std::unique_ptr<Move>>& neighbors
    );

private:
    // Helper method for drop down movements
    BlockPos getLandingBlock(Move const& node, int dx, int dz) const;
    bool     isSafeToBreak(BlockPos const& pos) const;
    bool     wouldCreateFlow(BlockPos const& pos) const;
    double   calculateMoveCost(Move const& from, Move const& to) const;
};

} // namespace we::bot