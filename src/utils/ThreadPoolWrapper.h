#pragma once

#include <ll/api/thread/ThreadPool.h>
#include <ll/api/thread/TickSyncTaskPool.h>
#include <mc/deps/core/common/bedrock/AssignedThread.h>
#include <mc/deps/core/common/bedrock/Threading.h>

namespace we {
class ThreadPoolWrapper {
    ll::thread::ThreadPool& pool;

public:
    ThreadPoolWrapper(ll::thread::ThreadPool& pool) : pool(pool) {}
    template <class F, class... Args>
    decltype(auto) addTask(F&& f, Args&&... args) {
        return pool.addTask(std::forward<F>(f), std::forward<Args>(args)...);
    }
};
class TickPoolWrapper {
    ll::thread::TickSyncTaskPool& pool;

public:
    TickPoolWrapper(ll::thread::TickSyncTaskPool& pool) : pool(pool) {}
    template <class F, class... Args>
    decltype(auto) addTask(F&& f, Args&&... args) {
        if (Bedrock::Threading::getMainThread().isOnThread()
            || Bedrock::Threading::getServerThread().isOnThread()) {
            auto task =
                std::make_shared<std::packaged_task<std::invoke_result_t<F, Args...>()>>(
                    [f = std::forward<F>(f), args...] { return f(args...); }
                );
            auto res = task->get_future();
            (*task)();
            return res;
        } else {
            return pool.addTask(std::forward<F>(f), std::forward<Args>(args)...);
        }
    }
};
} // namespace we
