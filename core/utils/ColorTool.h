//
// Created by OEOTYAN on 2023/02/01.
//

#pragma once

#include "Global.h"

namespace worldedit {
    mce::Color linearLerp(const mce::Color& k, const mce::Color& l, float m);
    mce::Color mixboxLerp(const mce::Color& k, const mce::Color& l, float m);
    mce::Color linearAverage(std::unordered_map<mce::Color, int> colorWithWeight);
    mce::Color mixboxAverage(std::unordered_map<mce::Color, int> colorWithWeight);

    float clamp(float x, float lowerlimit, float upperlimit);

    float smoothstep(float edge0, float edge1, float x);

    float smootherstep(float edge0, float edge1, float x);

    float smoothBrushAlpha(float r, float density, float opacity, unsigned short size);
}