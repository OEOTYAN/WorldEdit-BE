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
class SimulatedPlayerBuilder;

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
    std::function<void(bool, BlockTask&)> callback; // Called when task completes
    int                                   retryCount;
    static constexpr int                  MAX_RETRIES = 6;

    BlockTask(
        BlockPos                    pos,
        Block const*                block,
        DimensionType               dimension,
        std::shared_ptr<BlockActor> blockActor = nullptr,
        TaskPriority                priority   = TaskPriority::Normal,
        std::function<void(bool, BlockTask&)>   callback   = nullptr
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
        if(a->pos.y != b->pos.y){
            return a->pos.y > b->pos.y; // Lower Y first
        }
        return a->createdTime > b->createdTime; // FIFO for same priority
    }
};

// SimulatedPlayer wrapper for task management
struct ManagedSimulatedPlayer
: public std::enable_shared_from_this<ManagedSimulatedPlayer> {
    SimulatedPlayer*                      player;
    DimensionType                         dimension;
    std::shared_ptr<bot::BotPathfinder>   pathfinder;
    std::shared_ptr<BlockTask>            currentTask;
    std::chrono::steady_clock::time_point lastActivityTime;
    bool                                  isIdle;

    ManagedSimulatedPlayer(
        SimulatedPlayer*        player,
        DimensionType           dimension,
        SimulatedPlayerBuilder* builder
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

    // Task queue management
    std::priority_queue<
        std::shared_ptr<BlockTask>,
        std::vector<std::shared_ptr<BlockTask>>,
        TaskComparator>
        mTaskQueue;

    // SimulatedPlayer pool management
    ll::SmallDenseMap<size_t, std::shared_ptr<ManagedSimulatedPlayer>> mPlayerPool;
    ll::SmallDenseSet<size_t> mUnusedPlayerIndices;

    // Settings
    std::chrono::milliseconds mPlayerIdleTimeout;
    size_t                    mCleanupCounter = 0;

    // Statistics
    std::atomic<size_t> mTasksCompleted;
    std::atomic<size_t> mTasksFailed;

public:
    std::atomic_bool                 mIsRunning;
    ll::ConcurrentDenseSet<BlockPos> mDonePositions; // To avoid break blocks

    std::mutex taskMutex;
    std::vector<std::pair<std::shared_ptr<BlockTask>, std::shared_ptr<ManagedSimulatedPlayer>>> mPendingFailures;

    SimulatedPlayerBuilder(LocalContext& context, size_t maxPlayers = 8);
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
        std::function<void(bool, BlockTask&)>   callback   = nullptr
    );

    // Task queue management
    void   addTask(std::shared_ptr<BlockTask> task);
    void   clearTasks();
    size_t getQueuedTaskCount() const { return mTaskQueue.size(); }
    size_t getCompletedTaskCount() const { return mTasksCompleted.load(); }
    size_t getFailedTaskCount() const { return mTasksFailed.load(); }

    // SimulatedPlayer pool management
    bool createSimulatedPlayer(DimensionType dimension = DimensionType{0});
    void removeIdlePlayers();
    void removeAllPlayers();

    // Configuration
    void setPlayerIdleTimeout(std::chrono::milliseconds timeout) {
        mPlayerIdleTimeout = timeout;
    }

    ll::coro::CoroTask<> processTasksAsync(); // Process tasks asynchronously

private:
    // Internal helper methods
    ll::coro::CoroTask<> assignTaskToPlayerAsync(
        std::shared_ptr<BlockTask> task,
        ManagedSimulatedPlayer*    managedPlayer
    );
    ll::coro::CoroTask<> processTaskAsync(ManagedSimulatedPlayer* managedPlayer);

    void handleTaskFailure(
        std::shared_ptr<BlockTask> task,
        ManagedSimulatedPlayer*    managedPlayer
    );
    SimulatedPlayer*
    createSimulatedPlayerInDimension(const std::string& name, DimensionType dimension);
    ManagedSimulatedPlayer* findIdlePlayer(DimensionType dimension);
    void                    cleanupCompletedTasks();
    void                    removePlayer(size_t index);
};

} // namespace we