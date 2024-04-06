#pragma once

#include <vector>

namespace we {
std::vector<double> gaussianKernel(int size, double sigma = -1);
std::vector<double>
blur2D(std::vector<double> const& heightMap, int ksize, int sizex, int sizez);
} // namespace we
