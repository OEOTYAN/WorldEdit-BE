#include "LoftRegion.h"
#include "utils/Math.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {
LoftRegion::LoftRegion(DimensionType d, BoundingBox const& b)
: Region(d, (b.max + b.min) / 2) {}

ll::Expected<> LoftRegion::serialize(CompoundTag& tag) const {
    return Region::serialize(tag)
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["maxPointCount"], maxPointCount);
        })
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["loftPoints"], loftPoints);
        })
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["cycle"], cycle);
        });
}
ll::Expected<> LoftRegion::deserialize(CompoundTag const& tag) {
    return Region::deserialize(tag)
        .and_then([&, this]() {
            return ll::reflection::deserialize(maxPointCount, tag.at("maxPointCount"));
        })
        .and_then([&, this]() {
            return ll::reflection::deserialize(loftPoints, tag.at("loftPoints"));
        })
        .and_then([&, this]() {
            return ll::reflection::deserialize(cycle, tag.at("cycle"));
        });
}
void LoftRegion::updateBoundingBox() {
    posCached   = false;
    boundingBox = BoundingBox();
    interpolations.clear();
    verticalCurves.clear();
    std::vector<KochanekBartelsInterpolation> cache2;
    if (maxPointCount == 1) {
        std::vector<Node> nodes2;
        nodes2.clear();
        for (auto& point : loftPoints) {
            nodes2.push_back(Node(point.front()));
        }
        if (!nodes2.empty())
            verticalCurves.push_back(KochanekBartelsInterpolation(nodes2, cycle));
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
            verticalCurves.push_back(KochanekBartelsInterpolation(nodes2, cycle));
        }
        auto rSize = loftPoints.size() + cycle - 1;
        for (int i = 1; i <= rSize; ++i) {
            std::vector<Node> nodes2;
            nodes2.clear();
            for (int j = 0; j < maxPointCount; ++j) {
                double t = (i * 2 - 1) / (rSize * 2.0);
                nodes2.push_back(Node(verticalCurves[j * 2].getPosition(t)));
            }
            cache2.push_back(KochanekBartelsInterpolation(nodes2));
        }
    }
    auto& we = WorldEdit::getInstance();

    auto& geo = we.getGeo();

    std::vector<bsci::GeometryGroup::GeoId> ids;

    auto addline = [&](KochanekBartelsInterpolation const& line) {
        if (line.segCount == 0) {
            return;
        } else if (line.segCount == 1) {
            ids.emplace_back(geo.line(
                getDim(),
                line.getPosition(0.0),
                line.getPosition(1.0),
                we.getConfig().colors.region_line_color
            ));
        }
        std::vector<Vec3> points;
        for (int i = 0; i < line.segCount; i++) {
            size_t linenum =
                2 * std::clamp<size_t>((size_t)(line.arcLength(i) / 1.5), 1, 8);
            for (int j = 0; j < linenum; j++) {
                points.emplace_back(line.getPosition(i, j / (double)linenum));
            }
        }
        points.emplace_back(line.getPosition(1.0));
        ids.emplace_back(
            geo.line(getDim(), points, we.getConfig().colors.region_line_color)
        );
    };
    for (auto& line : interpolations) {
        addline(line);
    }
    for (auto& line : verticalCurves) {
        addline(line);
    }
    for (auto& line : cache2) {
        addline(line);
    }
    for (auto& points : loftPoints) {
        bool first{true};
        for (auto& point : points) {
            ids.emplace_back(geo.box(
                getDim(),
                point,
                first ? we.getConfig().colors.region_point_color2
                      : we.getConfig().colors.region_point_color
            ));
            first = false;
        }
    }
    views = geo.merge(ids);
}

bool LoftRegion::contains(BlockPos const& pos) const {
    if (posCached) {
        return posCache.contains(pos);
    }
    return false;
};

bool LoftRegion::removePoint(std::optional<BlockPos> const& pos) {
    if (!pos) {
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
                auto length = (loftPoints[i][j] - *pos).length();
                if (length <= minDis) {
                    minDis = length;
                    pi     = i;
                    pj     = j;
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

void LoftRegion::buildCache() const {
    if (posCached) {
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
    double      step = (1.0 / maxCurveLength) / quality;
    BoundingBox box(loftPoints[0][0], loftPoints[0][0]);
    for (double t = 0; t <= 1.0; t += step) {
        std::vector<Node> nodes;
        nodes.clear();
        for (auto& curve : interpolations) {
            nodes.push_back(Node(curve.getPosition(t)));
        }
        auto tmpCurve = KochanekBartelsInterpolation(nodes, cycle);
        auto step2    = (1.0 / tmpCurve.arcLength()) / quality;
        for (double t2 = 0; t2 <= 1.0; t2 += step2) {
            BlockPos tmpPos  = tmpCurve.getPosition(t2);
            posCache[tmpPos] = std::make_pair(1 - t2, 1 - t);
            box              = box.merge(tmpPos);
        }
    }
    boundingBox = box;
}

BoundingBox LoftRegion::getBoundBox() const {
    buildCache();
    return boundingBox;
}

void LoftRegion::forEachBlockInRegion(std::function<void(BlockPos const&)>&& todo) const {
    buildCache();
    for (auto& pos : posCache) {
        todo(pos.first);
    }
}

void LoftRegion::forEachBlockUVInRegion(
    std::function<void(BlockPos const&, double, double)>&& todo
) const {
    buildCache();
    for (auto& pos : posCache) {
        todo(pos.first, pos.second.first, pos.second.second);
    }
}

void LoftRegion::forEachBlockInLines(
    int                                         num,
    bool                                        isX,
    const std::function<void(BlockPos const&)>& todo
) {
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

                auto tmpCurve = KochanekBartelsInterpolation(nodes2, cycle);
                auto step2    = (1.0 / tmpCurve.arcLength()) / quality;
                for (double t2 = 0; t2 <= 1.0; t2 += step2) {
                    posCache2.insert(tmpCurve.getPosition(t2));
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
            double t = i / (double)(num - 1 + cycle);
            for (size_t endj = verticalCurves.size(), j = 0; j < endj; j += 2) {
                auto& curve = verticalCurves[j];
                nodes2.push_back(Node(curve.getPosition(t)));
            }
            auto tmpCurve = KochanekBartelsInterpolation(nodes2);
            auto step2    = (1.0 / tmpCurve.arcLength()) / quality;
            for (double t2 = 0; t2 <= 1.0; t2 += step2) {
                posCache2.insert(tmpCurve.getPosition(t2));
            }
        }

        for (auto& pos : posCache2) {
            todo(pos);
        }
        return;
    }
}
bool LoftRegion::setMainPos(BlockPos const& pos) {
    loftPoints.push_back({pos});
    updateBoundingBox();
    return true;
}
bool LoftRegion::setOffPos(BlockPos const& pos) {
    if (loftPoints.empty()) {
        return false;
    }
    loftPoints.back().push_back(pos);
    maxPointCount = std::max(maxPointCount, loftPoints.back().size());
    updateBoundingBox();
    return true;
}
bool LoftRegion::shift(BlockPos const& change) {
    for (auto& points : loftPoints) {
        for (auto& point : points) {
            point += change;
        }
    }
    updateBoundingBox();
    return true;
}

} // namespace we
