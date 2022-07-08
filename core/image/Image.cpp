//
// Created by OEOTYAN on 2022/06/11.
//
#include "Image.h"
#include "eval/Eval.h"
#include "MC/Level.hpp"
namespace worldedit {
    double Sampler::setUV(double u) const {
        switch (edgeType) {
            case EdgeType::CLAMP:
                return std::min(1.0, std::max(0.0, u));
            case EdgeType::REPEAT:
                return u - floor(u);
            case EdgeType::FLIP:
                return abs(1 - posfmod(u + 1, 2));
            default:
                return -1;
        }
    }

    Texture2D::Texture2D(unsigned w, unsigned h) : width(w), height(h), size((unsigned long long)(w)*h) {
        rawColor.resize((unsigned long long)(w)*h);
    };

    mce::Color Texture2D::load(const Sampler& sampler, double u, double v, double offsetu, double offsetv) const {
        if (width > 1)
            u += offsetu / (width - 1);
        if (height > 1)
            v += offsetv / (height - 1);
        u = sampler.setUV(u);
        v = sampler.setUV(v);
        if (u < 0 || v < 0) {
            return mce::Color(0, 0, 0, 0);
        }
        u *= width - 1;
        v *= height - 1;
        u += 0.5;
        v += 0.5;
        unsigned iu = static_cast<unsigned>(floor(u));
        unsigned iv = static_cast<unsigned>(floor(v));
        return rawColor[iu + iv * width];
    }
    mce::Color Texture2D::sample(const Sampler& sampler, double u, double v, double lod) const {
        switch (sampler.samplerType) {
            case SamplerType::Trilinear:
            case SamplerType::Bilinear: {
                auto color0 = load(sampler, u, v);
                auto color1 = load(sampler, u, v, 1);
                auto color2 = load(sampler, u, v, 0, 1);
                auto color3 = load(sampler, u, v, 1, 1);

                mce::Color res(0, 0, 0, 0);
                u *= width - 1;
                v *= height - 1;
                u += 0.5;
                v += 0.5;
                u -= floor(u);
                v -= floor(v);

                double pm0 = (1 - u) * (1 - v);
                double pm1 = v * (1 - u);
                double pm2 = u * (1 - v);
                double pm3 = u * v;

                res.r = static_cast<float>(pm0 * color0.r + pm1 * color1.r + pm2 * color2.r + pm3 * color3.r);
                res.g = static_cast<float>(pm0 * color0.g + pm1 * color1.g + pm2 * color2.g + pm3 * color3.g);
                res.b = static_cast<float>(pm0 * color0.b + pm1 * color1.b + pm2 * color2.b + pm3 * color3.b);
                res.a = static_cast<float>(pm0 * color0.a + pm1 * color1.a + pm2 * color2.a + pm3 * color3.a);
                return res;
            }
            default:
                return load(sampler, u, v);
        }
    }
    Texture2D loadpng(const std::string& filename) {
        std::vector<unsigned char> image;

        unsigned width, height;
        unsigned error = lodepng::decode(image, width, height, filename);
        if (error != 0) {
            //std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
            Level::broadcastText("§c" + std::string(lodepng_error_text(error)), TextType::RAW);
            return Texture2D(0, 0);
        }
        Texture2D res(width, height);
        for (size_t v = 0; v < height; v++)
            for (size_t u = 0; u < width; u++) {
                auto k            = width * v + u;
                res.rawColor[k].r = image[4 * k] / 255.0f;
                res.rawColor[k].g = image[4 * k + 1] / 255.0f;
                res.rawColor[k].b = image[4 * k + 2] / 255.0f;
                res.rawColor[k].a = image[4 * k + 3] / 255.0f;
            }
        return res;
    }

    double colorToHeight(const mce::Color& color) {
        if (color > 0) {
            if (color.r == color.g && color.g == color.b) {
                return color.r;
            } else {
                auto colorLinear = color.sRGBToLinear();
                return colorLinear.r * 0.299 + colorLinear.g * 0.587 + colorLinear.b * 0.114;
            }
        }
        return 0;
    }

}  // namespace worldedit