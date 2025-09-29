#pragma once

#include "Goals.h"
#include "Heap.h"
#include "Move.h"
#include <chrono>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace we::bot {

class Movements; // Forward declaration

struct PathNode {
    std::unique_ptr<Move> data;
    double                g      = 0.0;
    double                h      = 0.0;
    double                f      = 0.0;
    PathNode*             parent = nullptr;

    PathNode() = default;
    PathNode(std::unique_ptr<Move> move, double g, double h, PathNode* parent = nullptr);

    // For heap comparison - we want the smallest f value
    bool operator<(PathNode const& other) const { return f < other.f; }
};

struct ComputedPath {
    enum class Status { Success, Failed };

    Status                             status;
    std::vector<std::unique_ptr<Move>> path;
    double                             cost = 0.0;

    ComputedPath(Status status) : status(status) {}
};

class AStar {
private:
    std::chrono::steady_clock::time_point mStartTime;
    Movements*                            mMovements;
    std::unique_ptr<Goal>                 mGoal;
    std::chrono::milliseconds             mTimeout;
    double                                mSearchRadius;

    std::unordered_set<BlockPos> mClosedDataSet;
    class NodeCompare {
    public:
        bool operator()(
            std::unique_ptr<PathNode> const& a,
            std::unique_ptr<PathNode> const& b
        ) const;
    };

    Heap<std::unique_ptr<PathNode>, NodeCompare> mOpenHeap;
    std::unordered_map<BlockPos, PathNode*>      mOpenDataMap;
    std::unique_ptr<PathNode>                    mBestNode;

    size_t mVisitedNodes = 0;

public:
    AStar(
        std::unique_ptr<Move>     start,
        Movements*                movements,
        std::unique_ptr<Goal>     goal,
        std::chrono::milliseconds timeout      = std::chrono::milliseconds(5000),
        double                    searchRadius = -1.0
    );

    ~AStar()                       = default;
    AStar(AStar const&)            = delete;
    AStar& operator=(AStar const&) = delete;
    AStar(AStar&&)                 = default;
    AStar& operator=(AStar&&)      = default;

    ll::coro::CoroTask<ComputedPath> computeAsync();

    PathNode* getBestNode() const { return mBestNode.get(); }
    size_t    getVisitedNodes() const { return mVisitedNodes; }

private:
    std::vector<std::unique_ptr<Move>> reconstructPath(PathNode* node);
    bool                      isInSearchRadius(Move const& move, Move const& start) const;
    std::chrono::milliseconds getElapsedTime() const;
};

} // namespace we::bot
