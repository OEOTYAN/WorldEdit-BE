#pragma once

#include <ll/api/thread/ThreadPool.h>

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
} // namespace we
