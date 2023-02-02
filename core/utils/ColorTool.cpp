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
    mce::Color linearAverage(std::unordered_map<mce::Color, int> colorWithWeight) {
        mce::Color finalColor;
        int colorCount = 0;
        for (auto& c : colorWithWeight) {
            if (colorCount != 0) {
                int lastColorCount = colorCount;
                colorCount += c.second;
                finalColor = linearLerp(c.first, finalColor, static_cast<float>(lastColorCount) / colorCount);
            } else {
                colorCount = c.second;
                finalColor = c.first;
            }
        }
        return finalColor;
    }
    mce::Color mixboxAverage(std::unordered_map<mce::Color, int> colorWithWeight) {
        mixbox_latent finallatent;
        int colorCount = 0;
        for (auto& c : colorWithWeight) {
            if (colorCount != 0) {
                int lastColorCount = colorCount;
                colorCount += c.second;
                float lerpK = static_cast<float>(lastColorCount) / colorCount;
                mixbox_latent tmplatent;

                mixbox_float_rgb_to_latent(c.first.r, c.first.g, c.first.b, tmplatent);

                for (int i = 0; i < MIXBOX_LATENT_SIZE; i++) {
                    finallatent[i] = std::lerp(tmplatent[i], finallatent[i], lerpK);
                }
            } else {
                colorCount = c.second;
                mixbox_float_rgb_to_latent(c.first.r, c.first.g, c.first.b, finallatent);
            }
        }
        mce::Color finalColor;
        mixbox_latent_to_float_rgb(finallatent, &finalColor.r, &finalColor.g, &finalColor.b);
        return finalColor;
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
        if (density == 1) {
            return opacity * (r <= size);
        }
        return opacity * smoothstep(size, size * density, r);
    }

}  // namespace worldedit