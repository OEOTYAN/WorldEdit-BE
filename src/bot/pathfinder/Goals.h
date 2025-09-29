#pragma once

#include "Move.h"
#include "worldedit/Global.h"
#include <memory>
#include <vector>

namespace we::bot {

class Goal {
public:
    virtual ~Goal() = default;

    // Return the distance between node and the goal
    virtual double heuristic(Move const& node) const = 0;

    // Return true if the node has reached the goal
    virtual bool isEnd(Move const& node) const = 0;

    // Return true if the goal has changed and the current path should be invalidated
    virtual bool hasChanged() const { return false; }

    // Returns true if the goal is still valid
    virtual bool isValid() const { return true; }

    // Clone this goal
    virtual std::unique_ptr<Goal> clone() const = 0;
};

// One specific block that the player should stand inside at foot level
class GoalBlock : public Goal {
public:
    int x, y, z;

    GoalBlock(int x, int y, int z);
    GoalBlock(BlockPos const& pos);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    std::unique_ptr<Goal> clone() const override;
};

// A block position that the player should get within a certain radius of
class GoalNear : public Goal {
public:
    int    x, y, z;
    double rangeSq;

    GoalNear(int x, int y, int z, double range);
    GoalNear(BlockPos const& pos, double range);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    std::unique_ptr<Goal> clone() const override;
};

// Useful for long-range goals that don't have a specific Y level
class GoalXZ : public Goal {
public:
    int x, z;

    GoalXZ(int x, int z);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    std::unique_ptr<Goal> clone() const override;
};

// Useful for finding builds that you don't have an exact Y level for
class GoalNearXZ : public Goal {
public:
    int    x, z;
    double rangeSq;

    GoalNearXZ(int x, int z, double range);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    std::unique_ptr<Goal> clone() const override;
};

// Get within a certain Y level
class GoalY : public Goal {
public:
    int y;

    GoalY(int y);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    std::unique_ptr<Goal> clone() const override;
};

// Follow another entity
class GoalFollow : public Goal {
private:
    Actor* mTarget;
    double mRange;
    Vec3   mLastPos;

public:
    GoalFollow(Actor* target, double range);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    bool                  hasChanged() const override;
    bool                  isValid() const override;
    std::unique_ptr<Goal> clone() const override;
};

// Reach any of multiple goals
class GoalComposite : public Goal {
private:
    std::vector<std::unique_ptr<Goal>> mGoals;

public:
    GoalComposite(std::vector<std::unique_ptr<Goal>> goals);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    bool                  hasChanged() const override;
    bool                  isValid() const override;
    std::unique_ptr<Goal> clone() const override;
};

// Invert a goal - avoid it instead of reaching it
class GoalInvert : public Goal {
private:
    std::unique_ptr<Goal> mGoal;

public:
    GoalInvert(std::unique_ptr<Goal> goal);

    double                heuristic(Move const& node) const override;
    bool                  isEnd(Move const& node) const override;
    bool                  hasChanged() const override;
    bool                  isValid() const override;
    std::unique_ptr<Goal> clone() const override;
};

} // namespace we::bot