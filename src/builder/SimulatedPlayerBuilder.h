#pragma once

#include "builder/Builder.h"
#include <mc/deps/core/utility/AutomaticID.h>
#include <mc/server/SimulatedPlayer.h>
#include <mc/world/level/dimension/Dimension.h>

// Type alias for DimensionType
using DimensionType = AutomaticID<Dimension, int>;

namespace we {

namespace bot {
class BotPathfinder; // Forward declaration
class Goal;          // Forward declaration
} // namespace bot

class LocalContext;

// Task priority levels
enum class TaskPriority { Low = 0, Normal = 1, High = 2, Urgent = 3 };

// Task status
enum class TaskStatus { Pending, InProgress, Completed, Failed, Cancelled };

// Block placement task
struct BlockTask {
    BlockPos                              pos;
    Block const*                          block;
    std::shared_ptr<BlockActor>           blockActor;
    DimensionType                         dimension;
    TaskPriority                          priority;
    TaskStatus                            status;
    std::chrono::steady_clock::time_point createdTime;
    std::chrono::steady_clock::time_point startTime;
    std::function<void(bool)>             callback; // Called when task completes
    int                                   retryCount;
    static constexpr int                  MAX_RETRIES = 128;

    BlockTask(
        BlockPos                    pos,
        Block const*                block,
        DimensionType               dimension,
        std::shared_ptr<BlockActor> blockActor = nullptr,
        TaskPriority                priority   = TaskPriority::Normal,
        std::function<void(bool)>   callback   = nullptr
    )
    : pos(pos),
      block(block),
      blockActor(blockActor),
      dimension(dimension),
      priority(priority),
      status(TaskStatus::Pending),
      createdTime(std::chrono::steady_clock::now()),
      callback(callback),
      retryCount(0) {}
};

// Compare tasks by priority and creation time
struct TaskComparator {
    bool operator()(
        const std::shared_ptr<BlockTask>& a,
        const std::shared_ptr<BlockTask>& b
    ) const {
        if (a->priority != b->priority) {
            return static_cast<int>(a->priority) < static_cast<int>(b->priority);
        }
        return a->createdTime > b->createdTime; // FIFO for same priority
    }
};

// SimulatedPlayer wrapper for task management
struct ManagedSimulatedPlayer {
    SimulatedPlayer*                      player;
    DimensionType                         dimension;
    std::unique_ptr<bot::BotPathfinder>   pathfinder;
    std::shared_ptr<BlockTask>            currentTask;
    std::chrono::steady_clock::time_point lastActivityTime;
    bool                                  isIdle;

    ManagedSimulatedPlayer(
        SimulatedPlayer*                     player,
        DimensionType                        dimension,
        std::function<bool(BlockPos const&)> customBlockCheck
    );
    ~ManagedSimulatedPlayer() = default;

    // Non-copyable but movable
    ManagedSimulatedPlayer(const ManagedSimulatedPlayer&)            = delete;
    ManagedSimulatedPlayer& operator=(const ManagedSimulatedPlayer&) = delete;
    ManagedSimulatedPlayer(ManagedSimulatedPlayer&&)                 = default;
    ManagedSimulatedPlayer& operator=(ManagedSimulatedPlayer&&)      = default;
};

class SimulatedPlayerBuilder : public Builder {
private:
    std::atomic_bool mIsRunning;

    ll::DenseSet<BlockPos> mPendingPositions; // To avoid break blocks

    // Task queue management
    std::priority_queue<
        std::shared_ptr<BlockTask>,
        std::vector<std::shared_ptr<BlockTask>>,
        TaskComparator>
        mTaskQueue;

    // SimulatedPlayer pool management
    std::vector<std::unique_ptr<ManagedSimulatedPlayer>> mPlayerPool;

    // Settings
    size_t                    mMaxPlayers;
    std::chrono::milliseconds mPlayerIdleTimeout;
    std::chrono::milliseconds mTaskTimeout;
    size_t                    mTasksPerTick;

    // Statistics
    std::atomic<size_t> mTasksCompleted;
    std::atomic<size_t> mTasksFailed;
    std::atomic<size_t> mActivePlayers;

public:
    SimulatedPlayerBuilder(LocalContext& context, size_t maxPlayers = 32);
    virtual ~SimulatedPlayerBuilder();

    void setup() override;

    void remove() override;

    // Builder interface - now queues tasks instead of immediate execution
    bool setBlock(
        BlockSource&,
        BlockPos const&,
        Block const&,
        std::shared_ptr<BlockActor>
    ) const override;

    // Enhanced block placement with priority and callback
    bool setBlockQueued(
        BlockSource&                blockSource,
        BlockPos const&             pos,
        Block const&                block,
        std::shared_ptr<BlockActor> blockActor = nullptr,
        TaskPriority                priority   = TaskPriority::Normal,
        std::function<void(bool)>   callback   = nullptr
    );

    // Task queue management
    void   addTask(std::shared_ptr<BlockTask> task);
    void   clearTasks();
    size_t getQueuedTaskCount() const { return mTaskQueue.size(); }
    size_t getCompletedTaskCount() const { return mTasksCompleted.load(); }
    size_t getFailedTaskCount() const { return mTasksFailed.load(); }

    // SimulatedPlayer pool management
    bool createSimulatedPlayer(
        const std::string& name,
        DimensionType      dimension = DimensionType{0}
    );
    void   removeIdlePlayers();
    void   removeAllPlayers();
    size_t getActivePlayerCount() const { return mActivePlayers.load(); }
    size_t getMaxPlayerCount() const { return mMaxPlayers; }
    void   setMaxPlayerCount(size_t maxPlayers) { mMaxPlayers = maxPlayers; }

    // Configuration
    void setPlayerIdleTimeout(std::chrono::milliseconds timeout) {
        mPlayerIdleTimeout = timeout;
    }
    void setTaskTimeout(std::chrono::milliseconds timeout) { mTaskTimeout = timeout; }
    void setTasksPerTick(size_t tasksPerTick) { mTasksPerTick = tasksPerTick; }

    // Player-specific pathfinding methods
    bot::BotPathfinder* getPlayerPathfinder(SimulatedPlayer* player);
    void                stopPlayerPathfinding(SimulatedPlayer* player);

    // Utility methods
    bool                 isPlayerPathfinding(SimulatedPlayer* player) const;
    Vec3                 getPlayerCurrentPosition(SimulatedPlayer* player) const;
    ll::coro::CoroTask<> processTasksAsync(); // Process tasks asynchronously

private:
    // Internal helper methods
    ll::coro::CoroTask<> assignTaskToPlayerAsync(
        std::shared_ptr<BlockTask> task,
        ManagedSimulatedPlayer*    managedPlayer
    );
    ll::coro::CoroTask<> processTaskAsync(ManagedSimulatedPlayer* managedPlayer);
    void                 handleTaskFailure(
                        std::shared_ptr<BlockTask> task,
                        ManagedSimulatedPlayer*    managedPlayer
                    );
    SimulatedPlayer*
    createSimulatedPlayerInDimension(const std::string& name, DimensionType dimension);
    ManagedSimulatedPlayer* findIdlePlayer(DimensionType dimension);
    void                    cleanupCompletedTasks();
    void                    removePlayer(size_t index);
    DimensionType           getDimensionFromBlockSource(BlockSource& blockSource) const;
};

} // namespace we