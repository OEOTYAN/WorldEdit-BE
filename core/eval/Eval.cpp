//
// Created by OEOTYAN on 2022/06/10.
//
#include "pcg_random.hpp"
#include "Eval.h"
namespace worldedit {
    double posfmod(double x, double y) {
        return x - floor(x / y) * y;
    }
    double uniformRandDouble() {
        static pcg32 rng(pcg_extras::seed_seq_from<std::random_device>{});
        static std::uniform_real_distribution<double> uniform_dist{0.0, 1.0};
        return uniform_dist(rng);
    }
    int getHighestTerrainBlock(BlockSource* blockSource, int x, int z, int minY, int maxY, std::string filter) {
        EvalFunctions f;
        f.setbs(blockSource);
        std::unordered_map<std::string, double> variables;
        for (int y = maxY; y >= minY; y--) {
            BlockPos pos(x, y, z);
            auto* block = &blockSource->getBlock(pos);
            if (block != BedrockBlocks::mAir) {
                if (filter == "") {
                    return y;
                } else {
                    variables["rx"] = pos.x;
                    variables["ry"] = pos.y;
                    variables["rz"] = pos.z;
                    if (cpp_eval::eval<double>(filter, variables, f) > 0.5) {
                        return y;
                    }
                }
            }
        }
        return minY - 1;
    }

}  // namespace worldedit
