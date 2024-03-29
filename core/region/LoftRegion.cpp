//
// Created by OEOTYAN on 2022/06/18.
//
#include "LoftRegion.h"

namespace worldedit {
    LoftRegion::LoftRegion(const BoundingBox& region, const int& dim) : Region(BoundingBox(), dim) {
        loftPoints.clear();
        interpolations.clear();
        cache1.clear();
        cache2.clear();
        regionType = LOFT;
    }

    void LoftRegion::updateBoundingBox() {
        posCached = false;
        boundingBox = BoundingBox();
        interpolations.clear();
        cache1.clear();
        cache2.clear();
        if (maxPointCount == 1) {
            std::vector<Node> nodes2;
            nodes2.clear();
            for (auto& point : loftPoints) {
                nodes2.push_back(Node(point[0]));
            }
            cache1.push_back(KochanekBartelsInterpolation(nodes2, circle));
        } else {
            for (auto& points : loftPoints) {
                std::vector<Node> nodes;
                nodes.clear();
                for (auto& point : points) {
                    nodes.push_back(Node(point));
                }
                interpolations.push_back(KochanekBartelsInterpolation(nodes));
            }

            for (int i = 0; i < maxPointCount * 2 - 1; ++i) {
                std::vector<Node> nodes2;
                nodes2.clear();
                double t = i / (double)((maxPointCount - 1) * 2);
                for (auto& curve : interpolations) {
                    nodes2.push_back(Node(curve.getPosition(t)));
                }
                cache1.push_back(KochanekBartelsInterpolation(nodes2, circle));
            }
            auto rSize = loftPoints.size() + circle - 1;
            for (int i = 1; i <= rSize; ++i) {
                std::vector<Node> nodes2;
                nodes2.clear();
                for (int j = 0; j < maxPointCount; ++j) {
                    double t = (i * 2 - 1) / (rSize * 2.0);
                    nodes2.push_back(Node(cache1[j * 2].getPosition(t)));
                }
                cache2.push_back(KochanekBartelsInterpolation(nodes2));
            }
        }
        rendertick = 0;
    }

    bool LoftRegion::contains(const BlockPos& pos) {
        if (posCached) {
            return posCache.contains(pos);
        }
        return false;
    };

    bool LoftRegion::removePoint(int dim, const BlockPos& pos) {
        if (!selecting) {
            return false;
        }
        if (dim != dimensionID) {
            return false;
        }
        if (loftPoints.size() == 1 && loftPoints[0].size() == 1) {
            selecting = false;
            return true;
        }
        if (pos == BlockPos::MIN) {
            auto& points = loftPoints.back();
            if (points.size() == 1) {
                loftPoints.pop_back();
            } else {
                points.pop_back();
            }
        } else {
            double minDis = DBL_MAX;
            size_t pi = 0, pj = 0;
            for (size_t endi = loftPoints.size(), i = 0; i < endi; ++i) {
                for (size_t endj = loftPoints[i].size(), j = 0; j < endj; ++j) {
                    auto length = (loftPoints[i][j] - pos).length();
                    if (length <= minDis) {
                        minDis = length;
                        pi = i;
                        pj = j;
                    }
                }
            }
            if (loftPoints[pi].size() == 1) {
                loftPoints.erase(loftPoints.begin() + pi);
            } else {
                loftPoints[pi].erase(loftPoints[pi].begin() + pj);
            }
        }
        updateBoundingBox();
        return true;
    }

    void LoftRegion::buildCache() {
        if (posCached) {
            return;
        }
        if (!selecting) {
            return;
        }
        if (interpolations.size() == 0) {
            return;
        }
        posCached = true;
        posCache.clear();
        double maxCurveLength = 0;
        for (auto& curve : interpolations) {
            maxCurveLength = std::max(maxCurveLength, curve.arcLength());
        }
        double step = (1.0 / maxCurveLength) / quality;
        BoundingBox box(loftPoints[0][0], loftPoints[0][0]);
        for (double t = 0; t <= 1.0; t += step) {
            std::vector<Node> nodes;
            nodes.clear();
            for (auto& curve : interpolations) {
                nodes.push_back(Node(curve.getPosition(t)));
            }
            auto tmpCurve = KochanekBartelsInterpolation(nodes, circle);
            auto step2 = (1.0 / tmpCurve.arcLength()) / quality;
            for (double t2 = 0; t2 <= 1.0; t2 += step2) {
                auto tmpPos = tmpCurve.getPosition(t2).toBlockPos();
                posCache[tmpPos] = std::make_pair(1 - t2, 1 - t);
                box = box.merge(tmpPos);
            }
        }
        boundingBox = box;
    }

    BoundingBox LoftRegion::getBoundBox() {
        buildCache();
        return boundingBox;
    }

    void LoftRegion::forEachBlockInRegion(const std::function<void(const BlockPos&)>& todo) {
        buildCache();
        for (auto& pos : posCache) {
            todo(pos.first);
        }
    }

    void LoftRegion::forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo) {
        buildCache();
        for (auto& pos : posCache) {
            todo(pos.first, pos.second.first, pos.second.second);
        }
    }

    void LoftRegion::forEachBlockInLines(int num, bool isX, const std::function<void(const BlockPos&)>& todo) {
        if (!selecting) {
            return;
        }
        if (num < 2) {
            return;
        }
        buildCache();
        if (isX) {
            if (loftPoints.size() == 1) {
                for (auto& pos : posCache) {
                    todo(pos.first);
                }
                return;
            } else {
                phmap::flat_hash_set<BlockPos> posCache2;
                for (int i = 0; i < num; ++i) {
                    std::vector<Node> nodes2;
                    nodes2.clear();
                    double t = i / (double)(num - 1);
                    for (auto& curve : interpolations) {
                        nodes2.push_back(Node(curve.getPosition(t)));
                    }

                    auto tmpCurve = KochanekBartelsInterpolation(nodes2, circle);
                    auto step2 = (1.0 / tmpCurve.arcLength()) / quality;
                    for (double t2 = 0; t2 <= 1.0; t2 += step2) {
                        auto tmpPos = tmpCurve.getPosition(t2).toBlockPos();
                        posCache2.insert(tmpPos);
                    }
                }
                for (auto& pos : posCache2) {
                    todo(pos);
                }
                return;
            }
        } else {
            phmap::flat_hash_set<BlockPos> posCache2;

            for (int i = 0; i < num; ++i) {
                std::vector<Node> nodes2;
                nodes2.clear();
                double t = i / (double)(num - 1 + circle);
                for (size_t endj = cache1.size(), j = 0; j < endj; j += 2) {
                    auto& curve = cache1[j];
                    nodes2.push_back(Node(curve.getPosition(t)));
                }
                auto tmpCurve = KochanekBartelsInterpolation(nodes2);
                auto step2 = (1.0 / tmpCurve.arcLength()) / quality;
                for (double t2 = 0; t2 <= 1.0; t2 += step2) {
                    auto tmpPos = tmpCurve.getPosition(t2).toBlockPos();
                    posCache2.insert(tmpPos);
                }
            }

            for (auto& pos : posCache2) {
                todo(pos);
            }
            return;
        }
    }

    void LoftRegion::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            rendertick = 40;
            for (auto& point : loftPoints) {
                globalPT().drawCuboid(point[0], dimensionID, mce::ColorPalette::SLATE);
                for (size_t endi = point.size(), i = 1; i < endi; ++i) {
                    globalPT().drawCuboid(point[i], dimensionID, mce::ColorPalette::GREEN);
                }
            }
            for (auto& curve : interpolations) {
                drawCurve(curve, mce::ColorPalette::YELLOW, dimensionID);
            }
            for (auto& curve : cache1) {
                drawCurve(curve, mce::ColorPalette::YELLOW, dimensionID);
            }
            for (auto& curve : cache2) {
                drawCurve(curve, mce::ColorPalette::YELLOW, dimensionID);
            }
        }
        rendertick--;
    }

    bool LoftRegion::setMainPos(const BlockPos& pos, const int& dim) {
        if (!selecting) {
            dimensionID = dim;
            selecting = true;
            loftPoints.clear();
            maxPointCount = 1;
        }
        if (dim != dimensionID) {
            return false;
        }
        std::vector<BlockPos> point;
        point.clear();
        point.push_back(pos);
        loftPoints.push_back(point);

        updateBoundingBox();
        return true;
    }
    bool LoftRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID) {
            return false;
        }
        (loftPoints.back()).push_back(pos);
        maxPointCount = std::max(maxPointCount, (int)(loftPoints.back().size()));

        updateBoundingBox();
        return true;
    }
    std::pair<std::string, bool> LoftRegion::shift(const BlockPos& change) {
        for (auto& points : loftPoints) {
            for (auto& point : points) {
                point += change;
            }
        }
        updateBoundingBox();
        return {"worldedit.shift.shifted", true};
    }

}  // namespace worldedit