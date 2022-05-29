#include "pcg_random.hpp"
#include "Eval.h"
namespace worldedit
{
    double uniformRandDouble() {
        static pcg32 rng(pcg_extras::seed_seq_from<std::random_device>{});
        static std::uniform_real_distribution<double> uniform_dist{0.0, 1.0};
        return uniform_dist(rng);
    }
} // namespace worldedit
