//
// Created by OEOTYAN on 2022/05/30.
//
#pragma once
#ifndef WORLDEDIT_BLUR_H
#define WORLDEDIT_BLUR_H

#include <vector>

namespace worldedit {
    std::vector<double> gaussianKernel(int size, double sigma = -1) {
        static const double small_gaussian_tab[][7] = {
            {1},
            {0.25, 0.5, 0.25},
            {0.062, 0.25, 0.375, 0.2, 0.0625},
            {0.03125, 0.109375, 0.21875, 0.28125, 0.21875, 0.109375, 0.03125}};
        std::vector<double> res;
        int ksize = size * 2 + 1;
        res.resize(size * 2 + 1);
        if (size < 4) {
            for (int i = 0; i < ksize; i++) {
                res[i] = small_gaussian_tab[size][i];
            }
            return res;
        }
        if (sigma < 0) {
            sigma = 0.3 * (0.5 * (size - 1) - 1) + 0.8;
        }
        double scale2 = -0.5 / (sigma * sigma);
        double sum = 0;
        for (int i = 0; i < ksize; i++) {
            double x = i - size;
            res[i] = exp(x * x * scale2);
            sum += res[i];
        }
        for (int i = 0; i < ksize; i++) {
            res[i] /= sum;
        }
        return res;
    }
}  // namespace worldedit

#endif  // WORLDEDIT_BLUR_H