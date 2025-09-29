#include "AStar.h"
#include "Movements.h"
#include <algorithm>
#include <cmath>
#include <coroutine>

#include "worldedit/WorldEdit.h"

namespace we::bot {

PathNode::PathNode(std::unique_ptr<Move> move, double g, double h, PathNode* parent)
: data(std::move(move)),
  g(g),
  h(h),
  f(g + h),
  parent(parent) {}


bool AStar::NodeCompare::operator()(
    std::unique_ptr<PathNode> const& a,
    std::unique_ptr<PathNode> const& b
) const {
    return a->f > b->f; // Min-heap based on f value
}

AStar::AStar(
    std::unique_ptr<Move>     start,
    Movements*                movements,
    std::unique_ptr<Goal>     goal,
    std::chrono::milliseconds timeout,
    double                    searchRadius
)
: mStartTime(std::chrono::steady_clock::now()),
  mMovements(movements),
  mGoal(std::move(goal)),
  mTimeout(timeout),
  mSearchRadius(searchRadius) {

    double h          = mGoal->heuristic(*start);
    auto   startNode  = std::make_unique<PathNode>(std::move(start), 0.0, h);
    mBestNode         = std::make_unique<PathNode>();
    mBestNode->data   = std::make_unique<Move>(*startNode->data);
    mBestNode->g      = startNode->g;
    mBestNode->h      = startNode->h;
    mBestNode->f      = startNode->f;
    mBestNode->parent = startNode->parent;

    PathNode* nodePtr = startNode.get();
    mOpenHeap.push(std::move(startNode));
    mOpenDataMap[*nodePtr->data] = nodePtr;
}

ll::coro::CoroTask<ComputedPath> AStar::computeAsync() {
    const int maxNodesBeforeYield = 32; // Process nodes in batches
    int       nodesSinceLastYield = 0;

    // Store processed nodes to maintain valid parent pointers
    std::vector<std::unique_ptr<PathNode>> processedNodes;

    while (!mOpenHeap.empty()) {

        // Check timeout
        auto now = std::chrono::steady_clock::now();
        if (now - mStartTime > mTimeout) {
            ComputedPath result(ComputedPath::Status::Failed);
            result.path = reconstructPath(mBestNode.get());
            result.cost = mBestNode ? mBestNode->g : 0.0;
            co_return result;
        }

        // Yield control periodically to avoid blocking
        if (nodesSinceLastYield >= maxNodesBeforeYield) {
            co_await ll::coro::yield; // Yield to other coroutines
            nodesSinceLastYield = 0;
        }

        auto currentNode = mOpenHeap.pop();
        if (!currentNode) {
            // Heap returned null/empty node, break
            break;
        }

        // mMovements->getPlayer()->simulateChat(
        //     fmt::format(
        //         "Pos:{},{},{}; OpenSet: {}, ClosedSet: {}, Visited: {}, BestH: {:.2f}, "
        //         "Elapsed: {}ms",
        //         currentNode->data->x,
        //         currentNode->data->y,
        //         currentNode->data->z,
        //         mOpenHeap.size(),
        //         mClosedDataSet.size(),
        //         mVisitedNodes,
        //         mBestNode->h,
        //         getElapsedTime().count()
        //     )
        // );

        // Remove from open set
        mOpenDataMap.erase(*currentNode->data);

        // Add to closed set
        mClosedDataSet.insert(*currentNode->data);
        mVisitedNodes++;
        nodesSinceLastYield++;

        // Check if we reached the goal
        if (mGoal->isEnd(*currentNode->data)) {
            ComputedPath result(ComputedPath::Status::Success);
            result.path = reconstructPath(currentNode.get());
            result.cost = currentNode->g;
            co_return result;
        }

        // Update best node if this one is closer to goal
        if (currentNode->h < mBestNode->h) {
            // mBestNode         = std::make_unique<PathNode>();
            mBestNode->data   = std::make_unique<Move>(*currentNode->data);
            mBestNode->g      = currentNode->g;
            mBestNode->h      = currentNode->h;
            mBestNode->f      = currentNode->f;
            mBestNode->parent = currentNode->parent;
        }

        // Get neighbors
        auto neighbors = mMovements->getNeighbors(*currentNode->data);

        for (auto& neighbor : neighbors) {

            // Skip if in closed set
            if (mClosedDataSet.count(*neighbor)) {
                continue;
            }

            // Skip if outside search radius
            if (mSearchRadius > 0 && !isInSearchRadius(*neighbor, *currentNode->data)) {
                continue;
            }

            double tentativeG = currentNode->g + neighbor->cost;

            // Check if this path to neighbor is better
            auto openIt = mOpenDataMap.find(*neighbor);
            if (openIt != mOpenDataMap.end()) {
                // Already in open set, check if this path is better
                if (tentativeG < openIt->second->g) {
                    openIt->second->g      = tentativeG;
                    openIt->second->f      = tentativeG + openIt->second->h;
                    openIt->second->parent = currentNode.get();
                }
            } else {
                // Not in open set, add it
                double h       = mGoal->heuristic(*neighbor);
                auto   newNode = std::make_unique<PathNode>(
                    std::move(neighbor),
                    tentativeG,
                    h,
                    currentNode.get()
                );
                PathNode* nodePtr = newNode.get();
                mOpenHeap.push(std::move(newNode));
                mOpenDataMap[*nodePtr->data] = nodePtr;
            }
        }

        // Store the processed node to keep parent pointers valid
        processedNodes.push_back(std::move(currentNode));
    }

    // No path found
    ComputedPath result(ComputedPath::Status::Failed);
    result.path = reconstructPath(mBestNode.get());
    result.cost = mBestNode ? mBestNode->g : 0.0;
    co_return result;
}

std::vector<std::unique_ptr<Move>> AStar::reconstructPath(PathNode* node) {
    std::vector<std::unique_ptr<Move>> path;
    while (node) {
        path.push_back(std::make_unique<Move>(*node->data));
        node = node->parent;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

bool AStar::isInSearchRadius(Move const& move, Move const& start) const {
    if (mSearchRadius <= 0) return true;

    double dx         = move.x - start.x;
    double dy         = move.y - start.y;
    double dz         = move.z - start.z;
    double distanceSq = dx * dx + dy * dy + dz * dz;

    return distanceSq <= mSearchRadius * mSearchRadius;
}

std::chrono::milliseconds AStar::getElapsedTime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - mStartTime);
}

} // namespace we::bot