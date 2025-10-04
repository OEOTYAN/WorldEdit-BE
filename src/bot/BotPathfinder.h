#pragma once

#include "builder/SimulatedPlayerBuilder.h"
#include "pathfinder/AStar.h"
#include "pathfinder/Goals.h"
#include "pathfinder/Move.h"
#include "pathfinder/Movements.h"
#include "utils/GeoContainer.h"
#include "worldedit/Global.h"
#include <chrono>
#include <functional>
#include <memory>

namespace we::bot {


class BotPathfinder : public std::enable_shared_from_this<BotPathfinder> {
private:
    bool initialized = false;
    bool running     = false;

    // Core pathfinding components
    std::vector<std::unique_ptr<Move>> mCurrentPath;

    bool                        debugPath = true;
    std::optional<GeoContainer> mDebugPathGeo;

    // Pathfinding state tracking
    bool mIsMining      = false;
    bool mIsBuilding    = false;
    bool mStopRequested = false;

    // Bot control state
    struct BotControlState {
        Vec3 localDirection = 0.0f;
        Vec3 target;
        bool jump   = false;
        bool sprint = false;
        bool sneak  = false;
    } mControlState;

    // Execution state
    bool   mIsExecuting      = false;
    bool   mFailToExecute    = false;
    size_t mCurrentPathIndex = 0;

    size_t mCurrentTick = 0;

    // Path deviation tracking
    size_t mLastProgressTick  = 0;
    size_t mLastProgressIndex = 0;

    // Path deviation thresholds
    static constexpr double MAX_DEVIATION_DISTANCE = 6.0; // Max distance from path
    static constexpr size_t PROGRESS_TIMEOUT{10 * 20};    // seconds without progress
    static constexpr size_t SCAFFOLDING_TIMEOUT{60 * 20}; // seconds to keep scaffolding

    // Associated objects
    SimulatedPlayer* mPlayer;

    Block const* mScaffoldingBlock = nullptr;

    std::function<bool(BlockPos const&)> mCustomBlockCheck;

    struct PlacedBlock {
        BlockPos pos;
        size_t tick;
       bool operator<(PlacedBlock const& other) const { return tick > other.tick; }
    };

    std::priority_queue<PlacedBlock> mRecentlyPlacedBlocks;

    public :
    // Configuration
    bool                      enablePathShortcut = false; // Path optimization
    bool LOSWhenPlacingBlocks = true; // Line of sight for block placement

    explicit BotPathfinder(
        SimulatedPlayer*                     player,
        std::function<bool(BlockPos const&)> customBlockCheck
    );
    ~BotPathfinder() = default;

    // Non-copyable but movable
    BotPathfinder(BotPathfinder const&)            = delete;
    BotPathfinder& operator=(BotPathfinder const&) = delete;
    BotPathfinder(BotPathfinder&&)                 = default;
    BotPathfinder& operator=(BotPathfinder&&)      = default;

    Block const* getScaffoldingBlock() const;

    // Path computation
    static ll::coro::CoroTask<ComputedPath> getPathToAsync(
        Vec3 const&                          start,
        Goal const&                          goal,
        BlockSource&                         bs,
        std::function<bool(BlockPos const&)> customBlockCheck
    );

    void initialize();

    void finalize();

    // Path execution
    void stop();
    void reset(std::string const& reason = "manual_reset");
    void setExecutingPath(std::vector<std::unique_ptr<Move>> path);

    bool failToExecute() const { return mFailToExecute; }
    void resetFailToExecute() { mFailToExecute = false; }

    // State queries
    bool                                      isExecuting() const { return mIsExecuting; }
    bool                                      isMining() const { return mIsMining; }
    bool                                      isBuilding() const { return mIsBuilding; }
    std::vector<std::unique_ptr<Move>> const& getCurrentPath() const {
        return mCurrentPath;
    }

    // Bot-specific functionality
    bool tick(); // Called every game tick to execute movement
    bool executeNextMove();
    void clearControlStates();

    enum class BreakBlockState { Fail = 0, Breaking = 1, Success = 2 };

    // SimulatedPlayer integration
    BreakBlockState breakBlock(BlockPos const& pos);
    bool            placeBlock(BlockPos const& pos, Block const* block);
    bool            equipBestTool(Block const* block);

    // Utility methods
    std::vector<std::unique_ptr<Move>>
    postProcessPath(std::vector<std::unique_ptr<Move>> path) const;

    // Configuration
    void setEnablePathShortcut(bool enable) { enablePathShortcut = enable; }

    // Player access methods
    Vec3             getCurrentPosition() const;
    BlockSource*     getBlockSource() const;
    Dimension*       getDimension() const;
    SimulatedPlayer* getPlayer() const { return mPlayer; }
    void             resetPlayer() { mPlayer = nullptr; }

private:
    // Core pathfinding methods
    void resetPath(std::string const& reason, bool clearStates = true);
    // Bot control methods
    void updateControlState(Move const& currentMove, Move const& nextMove);
    bool shouldJump(Move const& from, Move const& to) const;
    bool shouldSprint(Move const& from, Move const& to) const;
    bool needsToBreakBlocks(Move const& move) const;
    bool needsToPlaceBlocks(Move const& move) const;
    void applyControlStatesToPlayer();
    void updateControlStateForPosition(Vec3 const& targetPos);

    // Internal event handlers
    void onPathReset(std::string const& reason);
    void onGoalUpdated(Goal const* goal, bool dynamic);
    void onPathUpdated(std::vector<std::unique_ptr<Move>> const& path);
};

} // namespace we::bot