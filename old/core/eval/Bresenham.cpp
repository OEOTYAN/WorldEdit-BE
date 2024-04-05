//
// Created by OEOTYAN on 2022/06/13.
//

#include "Bresenham.hpp"
#include "Eval.h"
namespace worldedit {

    double getCatenaryParameter(double d, double h, double L) {
        const double m = sqrt(pow2(L) - pow2(h)) / d;
        double x = acosh(m) + 1;
        for (int i = 0; i < 10; i++) {
            x -= (sinh(x) - m * x) / (cosh(x) - m);
        }
        return d / (2 * x);
    }

    void plotLine(BlockPos const& pos0, BlockPos const& pos1, std::function<void(BlockPos const&)> const& todo) {
        int x0 = pos0.x;
        int y0 = pos0.y;
        int z0 = pos0.z;
        int x1 = pos1.x;
        int y1 = pos1.y;
        int z1 = pos1.z;
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int dz = abs(z1 - z0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int sz = z0 < z1 ? 1 : -1;

        int dMax = std::max(dx, std::max(dy, dz));
        if (dMax == dx) {
            for (int domstep = 0; domstep <= dx; domstep++) {
                int tipx = x0 + domstep * sx;
                int tipy = (int)round(y0 + domstep / ((double)dx) * ((double)dy) * sy);
                int tipz = (int)round(z0 + domstep / ((double)dx) * ((double)dz) * sz);
                todo(BlockPos(tipx, tipy, tipz));
            }
        } else if (dMax == dy) {
            for (int domstep = 0; domstep <= dy; domstep++) {
                int tipx = (int)round(x0 + domstep / ((double)dy) * ((double)dx) * sx);
                int tipy = y0 + domstep * sy;
                int tipz = (int)round(z0 + domstep / ((double)dy) * ((double)dz) * sz);
                todo(BlockPos(tipx, tipy, tipz));
            }
        } else {
            for (int domstep = 0; domstep <= dz; domstep++) {
                int tipx = (int)round(x0 + domstep / ((double)dz) * ((double)dx) * sx);
                int tipy = (int)round(y0 + domstep / ((double)dz) * ((double)dy) * sy);
                int tipz = z0 + domstep * sz;
                todo(BlockPos(tipx, tipy, tipz));
            }
        }
        return;
    }

    Vec3 KochanekBartelsInterpolation::retrieve(int index) const {
        if (index < 0) {
            if (circle) {
                return nodes[(int)(round(posfmod(index, (double)(nodes.size()))))].getVec3();
            } else {
                return nodes[0].getVec3();
            }
        } else if (index >= nodes.size()) {
            if (circle) {
                return nodes[(int)(round(posfmod(index, (double)(nodes.size()))))].getVec3();
            } else {
                return nodes[nodes.size() - 1].getVec3();
            }
        }
        return nodes[index].getVec3();
    }
    Vec3 KochanekBartelsInterpolation::linearCombination(int baseIndex,
                                                         double f1,
                                                         double f2,
                                                         double f3,
                                                         double f4) const {
        return retrieve(baseIndex - 1) * (float)f1 + retrieve(baseIndex) * (float)f2 +
               retrieve(baseIndex + 1) * (float)f3 + retrieve(baseIndex + 2) * (float)f4;
    }

    void KochanekBartelsInterpolation::recalc() {
        int nNodes = (int)nodes.size();
        if (circle) {
            nNodes += 1;
        }
        coeffA.resize(nNodes);
        coeffB.resize(nNodes);
        coeffC.resize(nNodes);
        coeffD.resize(nNodes);
        if (nNodes < 1) {
            return;
        }
        Node nodeB = nodes[0];
        double tensionB = nodeB.tension;
        double biasB = nodeB.bias;
        double continuityB = nodeB.continuity;
        for (int i = 0; i < nNodes; ++i) {
            const double tensionA = tensionB;
            const double biasA = biasB;
            const double continuityA = continuityB;
            if (i + 1 < nNodes) {
                if (circle) {
                    nodeB = nodes[(int)(round(posfmod(i + 1, (double)(nodes.size()))))];
                } else {
                    nodeB = nodes[i + 1];
                }
                tensionB = nodeB.tension;
                biasB = nodeB.bias;
                continuityB = nodeB.continuity;
            }

            const double ta = (1 - tensionA) * (1 + biasA) * (1 + continuityA) / 2;
            const double tb = (1 - tensionA) * (1 - biasA) * (1 - continuityA) / 2;
            const double tc = (1 - tensionB) * (1 + biasB) * (1 - continuityB) / 2;
            const double td = (1 - tensionB) * (1 - biasB) * (1 + continuityB) / 2;

            coeffA[i] = linearCombination(i, -ta, ta - tb - tc + 2, tb + tc - td - 2, td);
            coeffB[i] = linearCombination(i, 2 * ta, -2 * ta + 2 * tb + tc - 3, -2 * tb - tc + td + 3, -td);
            coeffC[i] = linearCombination(i, -ta, ta - tb, tb, 0);
            coeffD[i] = retrieve(i);
        }
        scaling = (int)nodes.size();
        if (!circle) {
            scaling -= 1;
        }
    }
    void KochanekBartelsInterpolation::setNodes(std::vector<Node> const& nodes) {
        this->nodes = nodes;
        recalc();
    }

    Vec3 KochanekBartelsInterpolation::getPosition(double position) const {
        position = std::max(0.0, std::min(position, 1.0));
        position *= scaling;
        int index = (int)position;
        float f = (float)(position - index);
        return ((coeffA[index] * f + coeffB[index]) * f + coeffC[index]) * f + coeffD[index];
    }

    Vec3 KochanekBartelsInterpolation::get1stDerivative(double position) const {
        position = std::max(0.0, std::min(position, 1.0));
        position *= scaling;
        int index = (int)position;
        return ((coeffA[index] * (float)(1.5 * position - 3.0 * index) + coeffB[index]) * (float)(2.0 * position) +
                (coeffA[index] * (float)(1.5 * index) - coeffB[index]) * (float)(2.0 * index) + coeffC[index]) *
               (float)scaling;
    }

    double KochanekBartelsInterpolation::arcLength(double positionA, double positionB) const {
        if (positionA > positionB) {
            std::swap(positionA, positionB);
        }
        positionA = std::max(0.0, std::min(positionA, 1.0));
        positionB = std::max(0.0, std::min(positionB, 1.0));
        positionA *= scaling;
        positionB *= scaling;
        int indexA = (int)positionA;
        int indexB = (int)positionB;
        double fA = positionA - indexA;
        double fB = positionB - indexB;
        return arcLengthRecursive(indexA, fA, indexB, fB);
    }

    double KochanekBartelsInterpolation::arcLengthRecursive(int indexLeft,
                                                            double remainderLeft,
                                                            int indexRight,
                                                            double remainderRight) const {
        switch (indexRight - indexLeft) {
            case 0:
                return arcLengthRecursive(indexLeft, remainderLeft, remainderRight);

            case 1:
                // This case is merely a speed-up for a very common case
                return arcLengthRecursive(indexLeft, remainderLeft, 1.0) +
                       arcLengthRecursive(indexRight, 0.0, remainderRight);

            default:
                return arcLengthRecursive(indexLeft, remainderLeft, indexRight - 1, 1.0) +
                       arcLengthRecursive(indexRight, 0.0, remainderRight);
        }
    }

    double KochanekBartelsInterpolation::arcLengthRecursive(int index,
                                                            double remainderLeft,
                                                            double remainderRight) const {
        auto a = coeffA[index] * 3;
        auto b = coeffB[index] * 2;
        auto c = coeffC[index];

        const int nPoints = 8;

        double accum = ((a * (float)remainderLeft + b) * (float)remainderLeft + c).length() * 0.5;
        for (int i = 0; i < nPoints - 1; ++i) {
            float t = (float)((double)i / (double)nPoints);
            t = (float)((remainderRight - remainderLeft) * t + remainderLeft);
            accum += ((a * t + b) * t + c).length();
        }
        accum += ((a * (float)remainderRight + b) * (float)remainderRight + c).length() * 0.5;
        return accum * (remainderRight - remainderLeft) / (double)nPoints;
    }

    int KochanekBartelsInterpolation::getSegment(double position) const {
        position = std::max(0.0, std::min(position, 1.0));
        position *= scaling;
        int index = (int)position;
        return index;
    }

}  // namespace worldedit