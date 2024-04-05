//
// Created by OEOTYAN on 2021/2/8.
//

#include "Globals.h"
#include "Region.h"

namespace worldedit {
    class SphereRegion final : public Region {
       private:
        BlockPos center = BlockPos::MIN;
        double radius = 0.5;
        int checkChanges(const std::vector<BlockPos>& changes);

       public:
        void updateBoundingBox() override;

        std::string getName() override { return "sphere"; }

        explicit SphereRegion(const BoundingBox& region, const int& dim);

        std::pair<std::string, bool> expand(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> contract(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        Vec3 getCenter() const override { return center.toVec3() + 0.5f; };

        uint64_t size() const override { return (uint64_t)std::round(4.0 / 3.0 * M_PI * radius * radius * radius); };

        void forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo) override;

        void renderRegion() override;

        double getRadius() const { return radius; };

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit