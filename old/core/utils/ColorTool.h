
#pragma once

#include "Globals.h"

namespace we {
mce::Color linearLerp(const mce::Color& k, const mce::Color& l, float m);
mce::Color mixboxLerp(const mce::Color& k, const mce::Color& l, float m);
mce::Color linearAverage(phmap::flat_hash_map<mce::Color, int> colorWithWeight);
mce::Color mixboxAverage(phmap::flat_hash_map<mce::Color, int> colorWithWeight);

float clamp(float x, float lowerlimit, float upperlimit);

float smoothstep(float edge0, float edge1, float x);

float smootherstep(float edge0, float edge1, float x);

float smoothBrushAlpha(float r, float density, float opacity, unsigned short size);
} // namespace we