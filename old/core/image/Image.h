//
// Created by OEOTYAN on 2022/06/11.
//
#pragma once

#include "Globals.h"
// #include "lodepng.h"

namespace worldedit {
    enum class SamplerType {
        Point = 0,
        Bilinear = 1,
        Bicubic = 2,
    };
    enum class EdgeType {
        ZERO = 0,
        CLAMP = 1,
        REPEAT = 2,
        FLIP = 3,
    };

    class Sampler {
       public:
        SamplerType samplerType = SamplerType::Point;
        EdgeType edgeType = EdgeType::REPEAT;

        Sampler() = default;
        Sampler(SamplerType s, EdgeType e) : samplerType(s), edgeType(e){};
        void setUV(double&) const;
    };

    class Texture2D {
       public:
        std::vector<mce::Color> rawColor;

        unsigned width = 0, height = 0;
        unsigned long long size = 0;

        Texture2D(unsigned w, unsigned h);
        mce::Color sample(const Sampler& sampler, double u, double v, double lod = 0.0) const;
        mce::Color load(const Sampler& sampler, double u, double v, double offsetu = 0.0, double offsetv = 0.0) const;
    };

    Texture2D loadImage(std::string const& filename);

    double colorToHeight(const mce::Color& color);
}  // namespace worldedit
