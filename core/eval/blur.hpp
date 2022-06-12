//
// Created by OEOTYAN on 2022/05/30.
//
#pragma once
#ifndef WORLDEDIT_BLUR_H
#define WORLDEDIT_BLUR_H

#include <vector>

namespace worldedit {
    std::vector<double> gaussianKernel(int size, double sigma = -1);
    std::vector<double> blur2D(std::vector<double> heightMap, int ksize, int sizex, int sizez);
}  // namespace worldedit

#endif  // WORLDEDIT_BLUR_H