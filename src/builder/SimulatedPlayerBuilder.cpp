#include "SimulatedPlayerBuilder.h"
#include "bot/BotPathfinder.h"
#include "bot/pathfinder/Goals.h"
#include "data/LocalContext.h"
#include "worldedit/WorldEdit.h"

namespace we {

SimulatedPlayerBuilder::SimulatedPlayerBuilder(LocalContext& context, size_t maxPlayers)
: Builder(context),
  mPlayerIdleTimeout(std::chrono::seconds(10)),
  mTasksCompleted(0),
  mTasksFailed(0) {
    for (size_t i = 0; i < maxPlayers; ++i) {
        mUnusedPlayerIndices.insert(i);
    }
}

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
    SimulatedPlayer*        player,
    DimensionType           dimension,
    SimulatedPlayerBuilder* builder
)
: player(player),
  dimension(dimension),
  isIdle(true),
  lastActivityTime(std::chrono::steady_clock::now()) {
    // Create individual pathfinder for this player
    pathfinder = std::make_shared<bot::BotPathfinder>(
        player,
        [this, builder](BlockPos const& pos) { return builder->isPositionDone(pos); }
    );
}

bool SimulatedPlayerBuilder::setBlock(
    BlockSource&                blockSource,
    BlockPos const&             pos,
    Block const&                block,
    std::shared_ptr<BlockActor> blockActor
) {
    auto& currentBlock = blockSource.getBlock(pos);
    if (&currentBlock == &block && !blockActor) {
        addDonePosition(pos, block.isAir());
        return true;
    }

    // Queue the task instead of immediate execution
    return setBlockQueued(
        blockSource,
        pos,
        block,
        std::move(blockActor),
        TaskPriority::Normal,
        [level = &blockSource.getLevel(), this](bool success, BlockTask& task) {
            if (success) {
                addDonePosition(task.pos, task.block->isAir());
            } else {
                auto dim = level->getDimension(task.dimension).lock();
                if (!dim) return;
                auto& bs = dim->getBlockSourceFromMainChunkSource();
                addDonePosition(task.pos, task.block->isAir());
                fallbackBuilder
                    .setBlock(bs, task.pos, *task.block, std::move(task.blockActor));
            }
        }
    );
}

bool SimulatedPlayerBuilder::setBlockQueued(
    BlockSource&                          blockSource,
    BlockPos const&                       pos,
    Block const&                          block,
    std::shared_ptr<BlockActor>           blockActor,
    TaskPriority                          priority,
    std::function<void(bool, BlockTask&)> callback
) {
    // Get dimension from BlockSource
    DimensionType dimension = blockSource.getDimensionId();

    // Create new task with dimension info
    auto task = std::make_shared<BlockTask>(
        pos,
        &block,
        dimension,
        std::move(blockActor),
        priority,
        std::move(callback)
    );
    addTask(std::move(task));
    return true;
}

void SimulatedPlayerBuilder::addTask(std::shared_ptr<BlockTask> task) {
    if (!task) return;
    mTaskQueue.push(task);
}

void SimulatedPlayerBuilder::clearTasks() {
    {
        std::lock_guard lock(taskMutex);
        for (auto& [task, player] : mPendingFailures) {
            task->status = TaskStatus::Cancelled;
            if (task->callback) {
                task->callback(false, *task);
            }
        }
        mPendingFailures.clear();
    }
    // Clear the queue
    while (!mTaskQueue.empty()) {
        auto task = mTaskQueue.top();
        mTaskQueue.pop();

        task->status = TaskStatus::Cancelled;
        if (task->callback) {
            task->callback(false, *task);
        }
    }

    // Clear current tasks from players
    for (auto& [_, managedPlayer] : mPlayerPool) {
        if (managedPlayer->currentTask) {
            managedPlayer->currentTask->status = TaskStatus::Cancelled;
            if (managedPlayer->currentTask->callback) {
                managedPlayer->currentTask->callback(false, *managedPlayer->currentTask);
            }
            managedPlayer->currentTask.reset();
            managedPlayer->isIdle = true;
        }
    }
}

bool SimulatedPlayerBuilder::createSimulatedPlayer(DimensionType dimension) {
    if (mUnusedPlayerIndices.empty()) {
        return false;
    }
    auto idx = *mUnusedPlayerIndices.begin();
    // Create SimulatedPlayer in the specified dimension
    auto* player = createSimulatedPlayerInDimension(
        fmt::format("BotBuilder_{}_{}", context.getUuid().asString().substr(0, 4), idx),
        dimension
    );
    if (!player) {
        return false;
    }
    mUnusedPlayerIndices.erase(mUnusedPlayerIndices.begin());

    auto managedPlayer =
        std::make_shared<ManagedSimulatedPlayer>(player, dimension, this);
    mPlayerPool[idx] = std::move(managedPlayer);

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
    player->setPlayerGameType(GameType::Survival);

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

    for (auto iter = mPlayerPool.begin(); iter != mPlayerPool.end();) {
        auto& [id, managedPlayer] = *iter;
        if (managedPlayer->isIdle
            && std::chrono::duration_cast<std::chrono::milliseconds>(
                   now - managedPlayer->lastActivityTime
               ) > mPlayerIdleTimeout) {
            removePlayer(id);
            iter = mPlayerPool.erase(iter);
        } else {
            ++iter;
        }
    }
}

void SimulatedPlayerBuilder::removeAllPlayers() {
    for (auto iter = mPlayerPool.begin(); iter != mPlayerPool.end();) {
        removePlayer(iter->first);
        iter = mPlayerPool.erase(iter);
    }
}


void SimulatedPlayerBuilder::removePlayer(size_t index) {
    auto& managedPlayer = mPlayerPool[index];
    // Cancel current task if any
    if (managedPlayer->currentTask) {
        handleTaskFailure(managedPlayer->currentTask, managedPlayer.get());
    }
    if (managedPlayer->player) {
        managedPlayer->player->kill();
        managedPlayer->player->simulateDisconnect();
        managedPlayer->player = nullptr;
    }
    managedPlayer->pathfinder->finalize();
    managedPlayer->pathfinder->resetPlayer();
    mUnusedPlayerIndices.insert(index);
}

ManagedSimulatedPlayer* SimulatedPlayerBuilder::findIdlePlayer(DimensionType dimension) {
    for (auto& [_, managedPlayer] : mPlayerPool) {
        if (managedPlayer->isIdle && managedPlayer->dimension == dimension) {
            return managedPlayer.get();
        }
    }
    return nullptr;
}

ll::coro::CoroTask<> SimulatedPlayerBuilder::processTasksAsync() {
    // Process tasks
    {
        std::lock_guard lock(taskMutex);
        for (auto& [task, player] : mPendingFailures) {
            handleTaskFailure(task, player.get());
        }
        mPendingFailures.clear();
    }

    for (auto iter = mPlayerPool.begin(); iter != mPlayerPool.end();) {
        auto& [id, managedPlayer] = *iter;
        if (!managedPlayer->player->isAlive() || managedPlayer->player->mRemoved) {
            removePlayer(id);
            iter = mPlayerPool.erase(iter);
        } else {
            ++iter;
        }
    }
    // Check completion of current tasks
    for (auto& [_, managedPlayer] : mPlayerPool) {
        if (managedPlayer->pathfinder->failToExecute()) {
            managedPlayer->pathfinder->resetFailToExecute();
            handleTaskFailure(managedPlayer->currentTask, managedPlayer.get());
        }
        if (!managedPlayer->isIdle && managedPlayer->currentTask) {
            co_await processTaskAsync(managedPlayer.get());
        }
    }

    std::vector<ll::coro::CoroTask<>> tasksToAwait;

    // Assign new tasks to idle players
    while (!mTaskQueue.empty()) {
        auto task = mTaskQueue.top();

        // Find an idle player in the same dimension as the task
        auto* idlePlayer = findIdlePlayer(task->dimension);
        if (!idlePlayer) {
            // Try to create a new player in the task's dimension if we have capacity
            if (createSimulatedPlayer(task->dimension)) {
                idlePlayer = findIdlePlayer(task->dimension);
            }
        }

        if (!idlePlayer) break; // No available player for this dimension

        mTaskQueue.pop();
        idlePlayer->isIdle = false;
        tasksToAwait.push_back(assignTaskToPlayerAsync(task, idlePlayer));
    }

    co_await ll::coro::collectAll(std::move(tasksToAwait));

    // Clean up idle players periodically
    if (++mCleanupCounter >= 100) { // Every ~5 seconds at 20 TPS
        mCleanupCounter = 0;
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
    auto& blockSource = managedPlayer->player->getDimensionBlockSource();
    auto  start       = managedPlayer->player->getFeetPos();

    managedPlayer->currentTask      = nullptr;
    managedPlayer->isIdle           = false;
    managedPlayer->lastActivityTime = std::chrono::steady_clock::now();

    task->status    = TaskStatus::InProgress;
    task->startTime = std::chrono::steady_clock::now();

    // Use pathfinding to navigate to the target position using player's own pathfinder
    auto goal =
        std::make_unique<bot::GoalNear>(task->pos.x, task->pos.y + 1, task->pos.z, 2.0);

    ll::coro::keepThis(
        [self = static_pointer_cast<SimulatedPlayerBuilder>(shared_from_this()),
         start,
         goal          = std::move(goal),
         blockSource   = blockSource.getWeakRef().lock(),
         managedPlayer = managedPlayer->shared_from_this(),
         task          = std::move(task)]() -> ll::coro::CoroTask<> {
            auto result = co_await bot::BotPathfinder::getPathToAsync(
                start,
                *goal,
                *blockSource,
                [&](BlockPos const& pos) { return self->isPositionDone(pos); }
            );
            if (!managedPlayer->player || self->mIsRunning == false) {
                co_return;
            }
            if (!result.path.empty()) {
                // Set the path for execution
                managedPlayer->pathfinder->setExecutingPath(std::move(result.path));
                managedPlayer->currentTask = task;
            } else {
                std::lock_guard lock(self->taskMutex);
                self->mPendingFailures.emplace_back(task, managedPlayer);
            }
        }
    ).launch(ll::thread::ThreadPoolExecutor::getDefault());
}

ll::coro::CoroTask<>
SimulatedPlayerBuilder::processTaskAsync(ManagedSimulatedPlayer* managedPlayer) {
    if (!managedPlayer || !managedPlayer->currentTask) {
        co_return;
    }
    managedPlayer->lastActivityTime = std::chrono::steady_clock::now();

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
            task->callback(true, *task);
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
            task->callback(false, *task);
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
} // namespace we