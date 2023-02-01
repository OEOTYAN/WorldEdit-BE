//
// Created by OEOTYAN on 2023/02/01.
//

#include "ColorTool.h"
#include "mixbox.h"

namespace worldedit {
    mce::Color linearLerp(const mce::Color& k, const mce::Color& l, float m) {
        return mce::Color::lerp(k.sRGBToLinear(), l.sRGBToLinear(), m).LinearTosRGB();
    }
    mce::Color mixboxLerp(const mce::Color& k, const mce::Color& l, float m) {
        mce::Color res;
        mixbox_lerp_float(k.r, k.g, k.b, l.r, l.g, l.b, m, &res.r, &res.g, &res.b);
        res.a = k.a * (1.0f - m) + l.a * m;
        return res;
    }

    float clamp(float x, float lowerlimit, float upperlimit) {
        if (x < lowerlimit)
            x = lowerlimit;
        if (x > upperlimit)
            x = upperlimit;
        return x;
    }

    float smoothstep(float edge0, float edge1, float x) {
        // Scale, and clamp x to 0..1 range
        x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);

        return x * x * (3 - 2 * x);
    }

    float smootherstep(float edge0, float edge1, float x) {
        // Scale, and clamp x to 0..1 range
        x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        // Evaluate polynomial
        return x * x * x * (x * (x * 6 - 15) + 10);
    }

    float smoothBrushAlpha(float r, float density, float opacity, unsigned short size) {
        return opacity * smoothstep(size, size * density, r);
    }

}  // namespace worldedit