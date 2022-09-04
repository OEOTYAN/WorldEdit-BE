//
// Created by OEOTYAN on 2022/05/30.
//
#include "blur.hpp"
#include "MC/Level.hpp"

namespace worldedit {

    std::vector<double> gaussianKernel(int size, double sigma) {
        static const double small_gaussian_tab[][7] = {
            {1},
            {0.25, 0.5, 0.25},
            {0.0625, 0.25, 0.375, 0.25, 0.0625},
            {0.03125, 0.109375, 0.21875, 0.28125, 0.21875, 0.109375, 0.03125}};
        std::vector<double> res;
        int ksize = size * 2 + 1;
        res.resize(size * 2 + 1);
        if (size < 4) {
            for (int i = 0; i < ksize; ++i) {
                res[i] = small_gaussian_tab[size][i];
            }
            return res;
        }
        if (sigma < 0) {
            sigma = 0.3 * (0.5 * (size - 1) - 1) + 0.8;
        }
        double scale2 = -0.5 / (sigma * sigma);
        double sum = 0;
        for (int i = 0; i < ksize; ++i) {
            double x = i - size;
            res[i] = exp(x * x * scale2);
            sum += res[i];
        }
        for (int i = 0; i < ksize; ++i) {
            res[i] /= sum;
        }
        return res;
    }
    std::vector<double> blur2D(std::vector<double> heightMap, int ksize, int sizex, int sizez) {
        std::vector<double> res;
        try {
            res.resize(heightMap.size());
        } catch (std::bad_alloc) {
            Level::broadcastText("Out of memory", TextType::RAW);
            return std::vector<double>(0);
        }
        auto kernel = gaussianKernel(ksize);
        for (int x = 0; x < sizex; ++x)
            for (int z = 0; z < sizez; ++z) {
                double sum = 0;
                double sumk = 0;
                for (int k = -ksize; k <= ksize; k++)
                    if (x + k >= 0 && x + k < sizex && heightMap[(x + k) * sizez + z] > -1e100) {
                        sum += heightMap[(x + k) * sizez + z] * kernel[k + ksize];
                        sumk += kernel[k + ksize];
                    }
                if (sumk <= 0) {
                    res[x * sizez + z] = heightMap[x * sizez + z];
                } else {
                    res[x * sizez + z] = sum / sumk;
                }
            }
        heightMap.assign(res.begin(), res.end());
        for (int x = 0; x < sizex; ++x)
            for (int z = 0; z < sizez; ++z) {
                double sum = 0;
                double sumk = 0;
                for (int k = -ksize; k <= ksize; k++)
                    if (z + k >= 0 && z + k < sizez && heightMap[x * sizez + (z + k)] > -1e100) {
                        sum += heightMap[x * sizez + (z + k)] * kernel[k + ksize];
                        sumk += kernel[k + ksize];
                    }
                if (sumk <= 0) {
                    res[x * sizez + z] = heightMap[x * sizez + z];
                } else {
                    res[x * sizez + z] = sum / sumk;
                }
            }
        return res;
    }
}  // namespace worldedit