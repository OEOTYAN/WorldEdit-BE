#pragma once
//
// Created by OEOTYAN on 2022/5/15.
//
#ifndef WORLDEDIT_EVAL_H
#define WORLDEDIT_EVAL_H

#include <numeric>
#include <vector>
#include <map>
#include <cmath>
#include <random>
#include <string>
#include "CppEval.h"
#include <MC/Actor.hpp>

namespace worldedit {
    class EvalFunctions {
       public:
        double operator()(const char* name, const std::vector<double>& params) {
            switch (do_hash(name)) {
                case do_hash("sin"):
                    if (params.size() == 1)
                        return sin(params[0]);
                    break;
                case do_hash("abs"):
                    if (params.size() == 1)
                        return abs(params[0]);
                    break;
                case do_hash("cos"):
                    if (params.size() == 1)
                        return cos(params[0]);
                    break;
                case do_hash("sign"):
                    if (params.size() == 1)
                        return params[0] == 0.0
                                   ? 0.0
                                   : (params[0] > 0.0 ? 1.0 : -1.0);
                    break;
                case do_hash("lg"):
                    if (params.size() == 1)
                        return log10(params[0]);
                    break;
                case do_hash("ln"):
                    if (params.size() == 1)
                        return log(params[0]);
                    break;
                case do_hash("log2"):
                    if (params.size() == 1)
                        return log2(params[0]);
                    break;
                case do_hash("round"):
                    if (params.size() == 1)
                        return round(params[0]);
                    break;
                case do_hash("floor"):
                    if (params.size() == 1)
                        return floor(params[0]);
                    break;
                case do_hash("ceil"):
                    if (params.size() == 1)
                        return ceil(params[0]);
                    break;
                case do_hash("exp"):
                    if (params.size() == 1)
                        return exp(params[0]);
                    break;
                case do_hash("exp2"):
                    if (params.size() == 1)
                        return exp2(params[0]);
                    break;
                case do_hash("sqrt"):
                    if (params.size() == 1)
                        return sqrt(params[0]);
                    break;
                case do_hash("tan"):
                    if (params.size() == 1)
                        return tan(params[0]);
                    break;
                case do_hash("atan"):
                    if (params.size() == 1)
                        return atan(params[0]);
                    break;
                case do_hash("atan2"):
                    if (params.size() == 2)
                        return atan2(params[0], params[1]);
                    break;
                case do_hash("asin"):
                    if (params.size() == 1)
                        return asin(params[0]);
                    break;
                case do_hash("acos"):
                    if (params.size() == 1)
                        return acos(params[0]);
                    break;
                case do_hash("sinh"):
                    if (params.size() == 1)
                        return sinh(params[0]);
                    break;
                case do_hash("cosh"):
                    if (params.size() == 1)
                        return cosh(params[0]);
                    break;
                case do_hash("tanh"):
                    if (params.size() == 1)
                        return tanh(params[0]);
                    break;
                case do_hash("gamma"):
                    if (params.size() == 1)
                        return tgamma(params[0]);
                    break;
                case do_hash("isslimechunk"):
                    if (params.size() == 2) {
                        int x = static_cast<int>(round(params[0]));
                        int z = static_cast<int>(round(params[1]));
                        auto seed = (x * 0x1f1f1f1fu) ^ (uint32_t)z;
                        std::mt19937 mt(seed);
                        return mt() % 10 == 0;
                    }
                    break;
                case do_hash("sum"):
                    return std::accumulate(params.begin(), params.end(), 0.0);
                    break;
                case do_hash("min"):
                    return *min_element(params.begin(), params.end());
                    break;
                case do_hash("max"):
                    return *max_element(params.begin(), params.end());
                default:
                    return 0;
            }
            return 0;
        }
    };

    double eval(Player* player, const std::string& s);
}  // namespace worldedit
#endif