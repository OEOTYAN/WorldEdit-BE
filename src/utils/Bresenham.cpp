#include "Bresenham.h"

namespace we {


Vec3 KochanekBartelsInterpolation::retrieve(int index) const {
    if (cycle) {
        return nodes[posmod(index, (int)nodes.size())];
    } else {
        return nodes[std::clamp(index, 0, (int)(nodes.size() - 1))];
    }
}
Vec3 KochanekBartelsInterpolation::linearCombination(
    int    baseIndex,
    double f1,
    double f2,
    double f3,
    double f4
) const {
    return retrieve(baseIndex - 1) * f1 + retrieve(baseIndex) * f2
         + retrieve(baseIndex + 1) * f3 + retrieve(baseIndex + 2) * f4;
}

void KochanekBartelsInterpolation::recalc() {
    int nNodes = (int)nodes.size();
    if (cycle) {
        nNodes += 1;
    }
    coeffA.resize(nNodes);
    coeffB.resize(nNodes);
    coeffC.resize(nNodes);
    coeffD.resize(nNodes);
    if (nNodes < 1) {
        return;
    }
    Node   nodeB       = nodes[0];
    double tensionB    = nodeB.tension;
    double biasB       = nodeB.bias;
    double continuityB = nodeB.continuity;
    for (int i = 0; i < nNodes; ++i) {
        double tensionA    = tensionB;
        double biasA       = biasB;
        double continuityA = continuityB;
        if (i + 1 < nNodes) {
            if (cycle) {
                nodeB = nodes[posmod(i + 1, (int)nodes.size())];
            } else {
                nodeB = nodes[i + 1];
            }
            tensionB    = nodeB.tension;
            biasB       = nodeB.bias;
            continuityB = nodeB.continuity;
        }

        double ta = (1 - tensionA) * (1 + biasA) * (1 + continuityA) / 2;
        double tb = (1 - tensionA) * (1 - biasA) * (1 - continuityA) / 2;
        double tc = (1 - tensionB) * (1 + biasB) * (1 - continuityB) / 2;
        double td = (1 - tensionB) * (1 - biasB) * (1 + continuityB) / 2;

        coeffA[i] = linearCombination(i, -ta, ta - tb - tc + 2, tb + tc - td - 2, td);
        coeffB[i] = linearCombination(
            i,
            2 * ta,
            -2 * ta + 2 * tb + tc - 3,
            -2 * tb - tc + td + 3,
            -td
        );
        coeffC[i] = linearCombination(i, -ta, ta - tb, tb, 0);
        coeffD[i] = retrieve(i);
    }
    segCount = (int)nodes.size();
    if (!cycle) {
        segCount -= 1;
    }
}
void KochanekBartelsInterpolation::setNodes(std::vector<Node> inNodes) {
    nodes = std::move(inNodes);
    recalc();
}

Vec3 KochanekBartelsInterpolation::getPosition(double position) const {
    position      = std::clamp(position, 0.0, 1.0);
    position     *= segCount;
    int    index  = (int)position;
    double f      = position - index;
    return getPosition(index, f);
}

Vec3 KochanekBartelsInterpolation::getPosition(int index, double position) const {
    return ((coeffA[index] * position + coeffB[index]) * position + coeffC[index])
             * position
         + coeffD[index];
}

Vec3 KochanekBartelsInterpolation::get1stDerivative(double position) const {
    position   = std::clamp(position, 0.0, 1.0);
    position  *= segCount;
    int index  = (int)position;
    return ((coeffA[index] * (1.5 * position - 3.0 * index) + coeffB[index])
                * (2.0 * position)
            + (coeffA[index] * (1.5 * index) - coeffB[index]) * (2.0 * index)
            + coeffC[index])
         * segCount;
}

double KochanekBartelsInterpolation::arcLength(double positionA, double positionB) const {
    if (positionA > positionB) {
        std::swap(positionA, positionB);
    }
    positionA      = std::clamp(positionA, 0.0, 1.0);
    positionB      = std::clamp(positionB, 0.0, 1.0);
    positionA     *= segCount;
    positionB     *= segCount;
    int    indexA  = (int)positionA;
    int    indexB  = (int)positionB;
    double fA      = positionA - indexA;
    double fB      = positionB - indexB;
    return arcLengthRecursive(indexA, fA, indexB, fB);
}

double KochanekBartelsInterpolation::arcLength(int index) const {
    return arcLengthRecursive(index, 0.0, 1.0);
}

double KochanekBartelsInterpolation::arcLengthRecursive(
    int    indexLeft,
    double remainderLeft,
    int    indexRight,
    double remainderRight
) const {
    switch (indexRight - indexLeft) {
    case 0:
        return arcLengthRecursive(indexLeft, remainderLeft, remainderRight);

    case 1:
        // This case is merely a speed-up for a very common case
        return arcLengthRecursive(indexLeft, remainderLeft, 1.0)
             + arcLengthRecursive(indexRight, 0.0, remainderRight);

    default:
        return arcLengthRecursive(indexLeft, remainderLeft, indexRight - 1, 1.0)
             + arcLengthRecursive(indexRight, 0.0, remainderRight);
    }
}

double KochanekBartelsInterpolation::arcLengthRecursive(
    int    index,
    double remainderLeft,
    double remainderRight
) const {
    auto a = coeffA[index] * 3;
    auto b = coeffB[index] * 2;
    auto c = coeffC[index];

    constexpr int nPoints = 8;

    double accum = ((a * remainderLeft + b) * remainderLeft + c).length() * 0.5;
    for (int i = 0; i < nPoints - 1; ++i) {
        double t  = ((double)i / (double)nPoints);
        t         = ((remainderRight - remainderLeft) * t + remainderLeft);
        accum    += ((a * t + b) * t + c).length();
    }
    accum += ((a * remainderRight + b) * remainderRight + c).length() * 0.5;
    return accum * (remainderRight - remainderLeft) / (double)nPoints;
}

int KochanekBartelsInterpolation::getSegment(double position) const {
    return (int)(std::clamp(position, 0.0, 1.0) * segCount);
}

} // namespace we
