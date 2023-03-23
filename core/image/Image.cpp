//
// Created by OEOTYAN on 2022/06/11.
//
#include "Image.h"
#include "eval/Eval.h"
#include "WorldEdit.h"
#include "mc/Level.hpp"
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
namespace worldedit {
    void Sampler::setUV(double& u) const {
        switch (edgeType) {
            case EdgeType::CLAMP:
                u = std::min(1.0, std::max(0.0, u));
                break;
            case EdgeType::REPEAT:
                u = u - floor(u);
                break;
            case EdgeType::FLIP:
                u = abs(1 - posfmod(u + 1, 2));
                break;
            default:
                u = 0;
                break;
        }
    }

    Texture2D::Texture2D(unsigned w, unsigned h) : width(w), height(h), size((unsigned long long)(w)*h) {
        rawColor.resize(size);
    };

    mce::Color Texture2D::load(const Sampler& sampler, double u, double v, double offsetu, double offsetv) const {
        u += offsetu / width;
        v += offsetv / height;
        sampler.setUV(u);
        sampler.setUV(v);
        if (u < 0 || v < 0) {
            return mce::Color(0, 0, 0, 0);
        }
        u *= width;
        v *= height;
        unsigned iu = std::min(width - 1, std::max(0u, static_cast<unsigned>(floor(u))));
        unsigned iv = std::min(height - 1, std::max(0u, static_cast<unsigned>(floor(v))));
        return rawColor[iu + iv * width];
    }
    mce::Color Texture2D::sample(const Sampler& sampler, double u, double v, double lod) const {
        switch (sampler.samplerType) {
            case SamplerType::Bicubic:
            case SamplerType::Bilinear: {
                auto color0 = load(sampler, u, v);
                auto color1 = load(sampler, u, v, 1);
                auto color2 = load(sampler, u, v, 0, 1);
                auto color3 = load(sampler, u, v, 1, 1);

                mce::Color res(0, 0, 0, 0);
                u *= width;
                v *= height;
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
    Texture2D loadImage(std::string const& filename) {
        int width, height, channel;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channel, 4);
        if (data == nullptr) {
            // logger().error("Image load error: {}", stbi_failure_reason());
            Level::broadcastText("§c" + tr("stb.image." + std::string(stbi_failure_reason())), TextType::RAW);
            return Texture2D(0, 0);
        }
        Texture2D res(width, height);
        for (size_t k = 0; k < res.size; k++) {
            auto k4 = 4 * k;
            res.rawColor[k] = mce::Color(data[k4], data[k4 + 1], data[k4 + 2], data[k4 + 3]);
        }
        stbi_image_free(data);
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