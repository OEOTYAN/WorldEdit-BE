#include "Goals.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace we::bot {

// Utility function for distance calculation
static double distanceXZ(double dx, double dz) { return std::sqrt(dx * dx + dz * dz); }

// GoalBlock implementation
GoalBlock::GoalBlock(int x, int y, int z)
: x(static_cast<int>(std::floor(x))),
  y(static_cast<int>(std::floor(y))),
  z(static_cast<int>(std::floor(z))) {}

GoalBlock::GoalBlock(BlockPos const& pos) : x(pos.x), y(pos.y), z(pos.z) {}

double GoalBlock::heuristic(Move const& node) const {
    double dx = x - node.x;
    double dy = y - node.y;
    double dz = z - node.z;
    return distanceXZ(dx, dz) + std::abs(dy);
}

bool GoalBlock::isEnd(Move const& node) const {
    return node.x == x && node.y == y && node.z == z;
}

std::unique_ptr<Goal> GoalBlock::clone() const {
    return std::make_unique<GoalBlock>(x, y, z);
}

// GoalNear implementation
GoalNear::GoalNear(int x, int y, int z, double range)
: x(static_cast<int>(std::floor(x))),
  y(static_cast<int>(std::floor(y))),
  z(static_cast<int>(std::floor(z))),
  rangeSq(range * range) {}

GoalNear::GoalNear(BlockPos const& pos, double range)
: x(pos.x),
  y(pos.y),
  z(pos.z),
  rangeSq(range * range) {}

double GoalNear::heuristic(Move const& node) const {
    double dx = x - node.x;
    double dy = y - node.y;
    double dz = z - node.z;
    return distanceXZ(dx, dz) + std::abs(dy);
}

bool GoalNear::isEnd(Move const& node) const {
    double dx = x - node.x;
    double dy = y - node.y;
    double dz = z - node.z;
    return (dx * dx + dy * dy + dz * dz) <= rangeSq;
}

std::unique_ptr<Goal> GoalNear::clone() const {
    return std::make_unique<GoalNear>(x, y, z, std::sqrt(rangeSq));
}

// GoalXZ implementation
GoalXZ::GoalXZ(int x, int z)
: x(static_cast<int>(std::floor(x))),
  z(static_cast<int>(std::floor(z))) {}

double GoalXZ::heuristic(Move const& node) const {
    double dx = x - node.x;
    double dz = z - node.z;
    return distanceXZ(dx, dz);
}

bool GoalXZ::isEnd(Move const& node) const { return node.x == x && node.z == z; }

std::unique_ptr<Goal> GoalXZ::clone() const { return std::make_unique<GoalXZ>(x, z); }

// GoalNearXZ implementation
GoalNearXZ::GoalNearXZ(int x, int z, double range)
: x(static_cast<int>(std::floor(x))),
  z(static_cast<int>(std::floor(z))),
  rangeSq(range * range) {}

double GoalNearXZ::heuristic(Move const& node) const {
    double dx = x - node.x;
    double dz = z - node.z;
    return distanceXZ(dx, dz);
}

bool GoalNearXZ::isEnd(Move const& node) const {
    double dx = x - node.x;
    double dz = z - node.z;
    return (dx * dx + dz * dz) <= rangeSq;
}

std::unique_ptr<Goal> GoalNearXZ::clone() const {
    return std::make_unique<GoalNearXZ>(x, z, std::sqrt(rangeSq));
}

// GoalY implementation
GoalY::GoalY(int y) : y(static_cast<int>(std::floor(y))) {}

double GoalY::heuristic(Move const& node) const { return std::abs(y - node.y); }

bool GoalY::isEnd(Move const& node) const { return node.y == y; }

std::unique_ptr<Goal> GoalY::clone() const { return std::make_unique<GoalY>(y); }

// GoalFollow implementation
GoalFollow::GoalFollow(Actor* target, double range)
: mTarget(target),
  mRange(range),
  mLastPos(target ? target->getFeetPos() : Vec3{0.0f, 0.0f, 0.0f}) {}

double GoalFollow::heuristic(Move const& node) const {
    if (!mTarget) return std::numeric_limits<double>::max();

    Vec3   targetPos = mTarget->getFeetPos();
    double dx        = targetPos.x - node.x;
    double dy        = targetPos.y - node.y;
    double dz        = targetPos.z - node.z;
    return distanceXZ(dx, dz) + std::abs(dy);
}

bool GoalFollow::isEnd(Move const& node) const {
    if (!mTarget) return false;

    Vec3   targetPos = mTarget->getFeetPos();
    double dx        = targetPos.x - node.x;
    double dy        = targetPos.y - node.y;
    double dz        = targetPos.z - node.z;
    double rangeSq   = mRange * mRange;
    return (dx * dx + dy * dy + dz * dz) <= rangeSq;
}

bool GoalFollow::hasChanged() const {
    if (!mTarget) return true;

    Vec3   currentPos = mTarget->getFeetPos();
    double dx         = currentPos.x - mLastPos.x;
    double dy         = currentPos.y - mLastPos.y;
    double dz         = currentPos.z - mLastPos.z;

    // Consider changed if target moved more than 0.5 blocks
    bool changed = (dx * dx + dy * dy + dz * dz) > 0.25;
    if (changed) {
        // We need to modify mLastPos but this is const method
        // Cast away const for this internal state update
        const_cast<GoalFollow*>(this)->mLastPos = currentPos;
    }
    return changed;
}

bool GoalFollow::isValid() const {
    return mTarget != nullptr; // Remove isValid() call as it doesn't exist in Actor
}

std::unique_ptr<Goal> GoalFollow::clone() const {
    return std::make_unique<GoalFollow>(mTarget, mRange);
}

// GoalComposite implementation
GoalComposite::GoalComposite(std::vector<std::unique_ptr<Goal>> goals)
: mGoals(std::move(goals)) {}

double GoalComposite::heuristic(Move const& node) const {
    double minHeuristic = std::numeric_limits<double>::max();
    for (auto const& goal : mGoals) {
        if (goal && goal->isValid()) {
            minHeuristic = std::min(minHeuristic, goal->heuristic(node));
        }
    }
    return minHeuristic == std::numeric_limits<double>::max() ? 0.0 : minHeuristic;
}

bool GoalComposite::isEnd(Move const& node) const {
    for (auto const& goal : mGoals) {
        if (goal && goal->isValid() && goal->isEnd(node)) {
            return true;
        }
    }
    return false;
}

bool GoalComposite::hasChanged() const {
    for (auto const& goal : mGoals) {
        if (goal && goal->hasChanged()) {
            return true;
        }
    }
    return false;
}

bool GoalComposite::isValid() const {
    for (auto const& goal : mGoals) {
        if (goal && goal->isValid()) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<Goal> GoalComposite::clone() const {
    std::vector<std::unique_ptr<Goal>> clonedGoals;
    clonedGoals.reserve(mGoals.size());
    for (auto const& goal : mGoals) {
        if (goal) {
            clonedGoals.push_back(goal->clone());
        }
    }
    return std::make_unique<GoalComposite>(std::move(clonedGoals));
}

// GoalInvert implementation
GoalInvert::GoalInvert(std::unique_ptr<Goal> goal) : mGoal(std::move(goal)) {}

double GoalInvert::heuristic(Move const& node) const {
    if (!mGoal || !mGoal->isValid()) return 0.0;

    // Invert the heuristic - we want to move away from the goal
    double distance = mGoal->heuristic(node);
    return distance == 0.0 ? std::numeric_limits<double>::max() : 1.0 / distance;
}

bool GoalInvert::isEnd(Move const& node) const {
    if (!mGoal || !mGoal->isValid()) return true;

    // We reach the inverted goal when we are NOT at the original goal
    return !mGoal->isEnd(node);
}

bool GoalInvert::hasChanged() const { return mGoal ? mGoal->hasChanged() : false; }

bool GoalInvert::isValid() const { return mGoal ? mGoal->isValid() : false; }

std::unique_ptr<Goal> GoalInvert::clone() const {
    if (mGoal) {
        return std::make_unique<GoalInvert>(mGoal->clone());
    }
    return nullptr;
}

} // namespace we::bot