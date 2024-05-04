#pragma once

#include "Math.h"

namespace we {
class Node : public Vec3 {
public:
    double tension    = 0;
    double bias       = 0;
    double continuity = 0;
    Node(Vec3 const& pos, double tension = 0, double bias = 0, double continuity = 0)
    : Vec3(pos),
      tension(tension),
      bias(bias),
      continuity(continuity) {}
    Node(BlockPos const& pos, double tension = 0, double bias = 0, double continuity = 0)
    : Node(pos.center(), tension, bias, continuity) {}

    Vec3 const& getVec3() const { return *this; }
};
class KochanekBartelsInterpolation {
public:
    std::vector<Node> nodes;
    std::vector<Vec3> coeffA;
    std::vector<Vec3> coeffB;
    std::vector<Vec3> coeffC;
    std::vector<Vec3> coeffD;
    int               segCount;
    bool              cycle = false;
    KochanekBartelsInterpolation(bool c = false) : cycle(c) {}
    KochanekBartelsInterpolation(std::vector<Node> n, bool c = false) : cycle(c) {
        setNodes(std::move(n));
    }
    Vec3   retrieve(int index) const;
    void   setNodes(std::vector<Node> nodes);
    Vec3   getPosition(double position) const;
    Vec3   getPosition(int index, double position) const;
    Vec3   get1stDerivative(double position) const;
    double arcLength(double positionA = 0, double positionB = 1) const;
    double arcLength(int index) const;
    int    getSegment(double position) const;

private:
    void recalc();
    Vec3
    linearCombination(int baseIndex, double f1, double f2, double f3, double f4) const;
    double arcLengthRecursive(
        int    indexLeft,
        double remainderLeft,
        int    indexRight,
        double remainderRight
    ) const;
    double
    arcLengthRecursive(int index, double remainderLeft, double remainderRight) const;
};
} // namespace we
