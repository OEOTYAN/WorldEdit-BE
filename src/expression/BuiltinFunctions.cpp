#include "expression/expression.h"

#include "FastNoiseLite.h"

#include <cmath>
#include <ll/api/utils/RandomUtils.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/Block.h>
#include <optional>
#include <string_view>

namespace we {
static std::optional<std::string_view>
getStringParam(ExprArgs params, std::size_t index) {
    if (index >= params.size()) {
        return std::nullopt;
    }

    using Store = exprtk::type_store<double>;
    if (params[index].type != Store::e_string) {
        return std::nullopt;
    }

    auto value = Store::string_view(params[index]);
    return std::string_view(value.begin(), value.size());
}

static double getScalarParam(ExprArgs params, std::size_t index) {
    if (index >= params.size()) {
        return 0.0;
    }
    return exprtk::igeneric_function<double>::generic_type::scalar_view(params[index]);
}

static bool iequals(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (std::tolower(lhs[i]) != std::tolower(rhs[i])) {
            return false;
        }
    }
    return true;
}

template <typename Enum>
struct EnumAlias {
    std::string_view key;
    Enum             value;
};

template <typename Enum, std::size_t N>
static std::optional<Enum> parseEnumAlias(
    std::optional<std::string_view> input,
    EnumAlias<Enum> const (&aliases)[N]
) {
    if (!input) {
        return std::nullopt;
    }

    for (auto const& alias : aliases) {
        if (iequals(*input, alias.key)) {
            return alias.value;
        }
    }
    return std::nullopt;
}

static std::optional<FastNoiseLite::FractalType>
parseFractalType(std::optional<std::string_view> input) {
    static constexpr EnumAlias<FastNoiseLite::FractalType> kAliases[] = {
        {"none",   FastNoiseLite::FractalType_None                 },
        {"fbm",    FastNoiseLite::FractalType_FBm                  },
        {"ridged", FastNoiseLite::FractalType_Ridged               },
        {"pp",     FastNoiseLite::FractalType_PingPong             },
        {"pgs",    FastNoiseLite::FractalType_DomainWarpProgressive},
        {"ind",    FastNoiseLite::FractalType_DomainWarpIndependent},
    };
    return parseEnumAlias(input, kAliases);
}

static std::optional<FastNoiseLite::CellularReturnType>
parseCellularReturnType(std::optional<std::string_view> input) {
    static constexpr EnumAlias<FastNoiseLite::CellularReturnType> kAliases[] = {
        {"cell", FastNoiseLite::CellularReturnType_CellValue   },
        {"val",  FastNoiseLite::CellularReturnType_CellValue   },
        {"dst",  FastNoiseLite::CellularReturnType_Distance    },
        {"dst2", FastNoiseLite::CellularReturnType_Distance2   },
        {"add",  FastNoiseLite::CellularReturnType_Distance2Add},
        {"sub",  FastNoiseLite::CellularReturnType_Distance2Sub},
        {"mul",  FastNoiseLite::CellularReturnType_Distance2Mul},
        {"div",  FastNoiseLite::CellularReturnType_Distance2Div},
    };
    return parseEnumAlias(input, kAliases);
}

static std::optional<FastNoiseLite::CellularDistanceFunction>
parseCellularDistanceFunction(std::optional<std::string_view> input) {
    static constexpr EnumAlias<FastNoiseLite::CellularDistanceFunction> kAliases[] = {
        {"eu",   FastNoiseLite::CellularDistanceFunction_Euclidean  },
        {"eusq", FastNoiseLite::CellularDistanceFunction_EuclideanSq},
        {"man",  FastNoiseLite::CellularDistanceFunction_Manhattan  },
        {"hyb",  FastNoiseLite::CellularDistanceFunction_Hybrid     },
    };
    return parseEnumAlias(input, kAliases);
}

static ExprValue makeNoiseResult(FastNoiseLite::NoiseType noiseType, ExprArgs params) {
    if (params.size() < 3) {
        return 0.0;
    }

    FastNoiseLite noise;
    noise.SetNoiseType(noiseType);
    if (params.size() >= 4) {
        noise.SetSeed(std::llround(getScalarParam(params, 3)));
    }
    if (params.size() >= 5) {
        if (auto fractal = parseFractalType(getStringParam(params, 4))) {
            noise.SetFractalType(*fractal);
        }
    }
    if (params.size() >= 6) {
        noise.SetFractalOctaves(std::llround(getScalarParam(params, 5)));
    }
    if (params.size() >= 7) {
        noise.SetFractalLacunarity(getScalarParam(params, 6));
    }
    if (params.size() >= 8) {
        noise.SetFractalGain(getScalarParam(params, 7));
    }
    if (params.size() >= 9) {
        noise.SetFractalWeightedStrength(getScalarParam(params, 8));
    }
    if (params.size() >= 10) {
        noise.SetFractalPingPongStrength(getScalarParam(params, 9));
    }

    return ExprValue(
        (noise.GetNoise(
             getScalarParam(params, 0),
             getScalarParam(params, 1),
             getScalarParam(params, 2)
         )
         + 1.0)
        * 0.5
    );
}

static ExprValue makeVoronoiResult(ExprArgs params) {
    if (params.size() < 3) {
        return 0.0;
    }

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    if (params.size() >= 4) {
        noise.SetSeed(std::llround(getScalarParam(params, 3)));
    }
    if (params.size() >= 5) {
        if (auto returnType = parseCellularReturnType(getStringParam(params, 4))) {
            noise.SetCellularReturnType(*returnType);
        }
    }
    if (params.size() >= 6) {
        if (auto distance = parseCellularDistanceFunction(getStringParam(params, 5))) {
            noise.SetCellularDistanceFunction(*distance);
        }
    }
    if (params.size() >= 7) {
        noise.SetCellularJitter(getScalarParam(params, 6));
    }
    if (params.size() >= 8) {
        if (auto fractal = parseFractalType(getStringParam(params, 7))) {
            noise.SetFractalType(*fractal);
        }
    }
    if (params.size() >= 9) {
        noise.SetFractalOctaves(std::llround(getScalarParam(params, 8)));
    }
    if (params.size() >= 10) {
        noise.SetFractalLacunarity(getScalarParam(params, 9));
    }
    if (params.size() >= 11) {
        noise.SetFractalGain(getScalarParam(params, 10));
    }
    if (params.size() >= 12) {
        noise.SetFractalWeightedStrength(getScalarParam(params, 11));
    }
    if (params.size() >= 13) {
        noise.SetFractalPingPongStrength(getScalarParam(params, 12));
    }

    return ExprValue(
        (noise.GetNoise(
             getScalarParam(params, 0),
             getScalarParam(params, 1),
             getScalarParam(params, 2)
         )
         + 1.0)
        * 0.5
    );
}

static optional_ref<Block const> getBlock(ExprCtx& ctx) {
    auto* blockSource = ctx.blockSource();
    if (!blockSource) {
        return nullptr;
    }
    return blockSource->getBlock(ctx.pos);
}
WE_EXPR_SCALAR(id, "Z")
[](ExprCtx& ctx, ExprArgs) {
    return getBlock(ctx)
        .transform([](Block const& block) {
            return static_cast<double>(block.getBlockItemId());
        })
        .value_or(0.0);
};

WE_EXPR_SCALAR(rtid, "Z")
[](ExprCtx& ctx, ExprArgs) {
    return getBlock(ctx)
        .transform([](Block const& block) {
            return static_cast<double>(block.mNetworkId);
        })
        .value_or(0.0);
};

WE_EXPR_SCALAR(rand, "Z|T|TT")
[](ExprCtx&, ExprArgs params) {
    if (params.empty()) {
        return ll::utils::random_utils::rand<double>();
    }

    if (params.size() == 1) {
        auto max = getScalarParam(params, 0);
        return ll::utils::random_utils::rand<double>(max);
    }

    auto min = getScalarParam(params, 0);
    auto max = getScalarParam(params, 1);
    if (max < min) {
        std::swap(min, max);
    }
    return ll::utils::random_utils::rand<double>(min, max);
};

WE_EXPR_SCALAR(simplex, "TTT|TTTT|TTTTS|TTTTST|TTTTSTT|TTTTSTTT|TTTTSTTTT|TTTTSTTTTT")
[](ExprCtx&, ExprArgs params) {
    return makeNoiseResult(FastNoiseLite::NoiseType_Simplex, params);
};

WE_EXPR_SCALAR(perlin, "TTT|TTTT|TTTTS|TTTTST|TTTTSTT|TTTTSTTT|TTTTSTTTT|TTTTSTTTTT")
[](ExprCtx&, ExprArgs params) {
    return makeNoiseResult(FastNoiseLite::NoiseType_Perlin, params);
};

WE_EXPR_SCALAR(cubic, "TTT|TTTT|TTTTS|TTTTST|TTTTSTT|TTTTSTTT|TTTTSTTTT|TTTTSTTTTT")
[](ExprCtx&, ExprArgs params) {
    return makeNoiseResult(FastNoiseLite::NoiseType_ValueCubic, params);
};

WE_EXPR_SCALAR(value, "TTT|TTTT|TTTTS|TTTTST|TTTTSTT|TTTTSTTT|TTTTSTTTT|TTTTSTTTTT")
[](ExprCtx&, ExprArgs params) {
    return makeNoiseResult(FastNoiseLite::NoiseType_Value, params);
};

WE_EXPR_SCALAR(
    voronoi,
    "TTT|TTTT|TTTTS|TTTTSS|TTTTSST|TTTTSSTS|TTTTSSTST|TTTTSSTSTT|TTTTSSTSTTT|"
    "TTTTSSTSTTTT|TTTTSSTSTTTTT"
)
[](ExprCtx&, ExprArgs params) { return makeVoronoiResult(params); };
} // namespace we
