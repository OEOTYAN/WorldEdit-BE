#pragma once

#include <algorithm>
#include <functional>
#include <vector>

namespace we::bot {

template <typename T, typename Compare = std::less<T>>
class Heap {
private:
    std::vector<T> mData;
    Compare        mCompare;

public:
    explicit Heap(Compare compare = Compare{}) : mCompare(compare) {}

    void push(T const& item) {
        mData.push_back(item);
        std::push_heap(mData.begin(), mData.end(), mCompare);
    }

    void push(T&& item) {
        mData.push_back(std::move(item));
        std::push_heap(mData.begin(), mData.end(), mCompare);
    }

    T pop() {
        if (mData.empty()) {
            return T{}; // Return default constructed value instead of throwing
        }

        std::pop_heap(mData.begin(), mData.end(), mCompare);
        T result = std::move(mData.back());
        mData.pop_back();
        return result;
    }

    T const& top() const {
        // Caller should check empty() before calling top()
        return mData.front();
    }

    bool empty() const { return mData.empty(); }

    size_t size() const { return mData.size(); }

    void clear() { mData.clear(); }
};

} // namespace we::bot