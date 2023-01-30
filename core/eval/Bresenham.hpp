//
// Created by OEOTYAN on 2022/06/14.
//
#pragma once
#include "Global.h"
namespace worldedit {
    double getCatenaryParameter(double d,double h,double L);

    void plotLine(BlockPos const& pos0,
                  BlockPos const& pos1,
                  std::function<void(class BlockPos const&)> const& todo);
    class Node : public Vec3 {
       public:
        double tension = 0;
        double bias = 0;
        double continuity = 0;
        Node(BlockPos const& pos,
             double tension = 0,
             double bias = 0,
             double continuity = 0)
            : Vec3(pos.toVec3() + 0.5),
              tension(tension),
              bias(bias),
              continuity(continuity) {}
        Node(Vec3 const& pos,
             double tension = 0,
             double bias = 0,
             double continuity = 0)
            : Vec3(pos), tension(tension), bias(bias), continuity(continuity) {}
        Vec3 getVec3() const { return Vec3(x, y, z); }
    };
    class KochanekBartelsInterpolation {
       public:
        std::vector<Node> nodes;
        std::vector<Vec3> coeffA;
        std::vector<Vec3> coeffB;
        std::vector<Vec3> coeffC;
        std::vector<Vec3> coeffD;
        double scaling;
        bool circle = false;
        KochanekBartelsInterpolation(bool c = false) : circle(c) {
            nodes.clear();
        }
        KochanekBartelsInterpolation(std::vector<Node> const& n, bool c = false)
            : circle(c) {
            nodes.clear();
            setNodes(n);
        }
        Vec3 retrieve(int index) const;
        void setNodes(std::vector<Node> const& nodes);
        Vec3 getPosition(double position) const;
        Vec3 get1stDerivative(double position) const;
        double arcLength(double positionA = 0, double positionB = 1) const;
        int getSegment(double position) const;

       private:
        void recalc();
        Vec3 linearCombination(int baseIndex,
                               double f1,
                               double f2,
                               double f3,
                               double f4) const;
        double arcLengthRecursive(int indexLeft,
                                  double remainderLeft,
                                  int indexRight,
                                  double remainderRight) const;
        double arcLengthRecursive(int index,
                                  double remainderLeft,
                                  double remainderRight) const;
    };
}  // namespace worldedit
