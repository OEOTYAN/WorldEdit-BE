//
// Created by OEOTYAN on 2022/5/15.
//
#pragma once
#include "Globals.h"
#include "Graphics.h"
#include "eval/Bresenham.hpp"

// #include "mc/Level.hpp"

#define MAX_LENGTH (64.0f)
namespace worldedit {
    class ::ParticleCUI& globalPT() {
        static ParticleCUI pt = ParticleCUI();
        return pt;
    }
    std::string facingToString(FACING facing) {
        switch (facing) {
            case FACING::POS_X:
                return "+x";
            case FACING::NEG_X:
                return "-x";
            case FACING::POS_Y:
                return "+y";
            case FACING::POS_Z:
                return "+z";
            case FACING::NEG_Y:
                return "-y";
            case FACING::NEG_Z:
                return "-z";
            default:
                return "unknown";
        }
    }

    std::string facingToDirString(FACING facing) {
        switch (facing) {
            case FACING::POS_X:
                return "west";
            case FACING::NEG_X:
                return "east";
            case FACING::POS_Y:
                return "up";
            case FACING::POS_Z:
                return "south";
            case FACING::NEG_Y:
                return "down";
            case FACING::NEG_Z:
                return "north";
            default:
                return "unknown";
        }
    }
    FACING dirStringToFacing(std::string_view dir) {
        if (dir == "west") {
            return FACING::POS_X;
        } else if (dir == "east") {
            return FACING::NEG_X;
        } else if (dir == "down") {
            return FACING::NEG_Y;
        } else if (dir == "south") {
            return FACING::POS_Z;
        } else if (dir == "north") {
            return FACING::NEG_Z;
        }
        return FACING::POS_Y;
    }
    FACING dirToFacing(const Vec3& dir) {
        Vec3 mDir = dir.normalize();
        if (mDir.x > 0.71) {
            return FACING::POS_X;
        } else if (mDir.x < -0.71) {
            return FACING::NEG_X;
        } else if (mDir.y < -0.71) {
            return FACING::NEG_Y;
        } else if (mDir.z > 0.71) {
            return FACING::POS_Z;
        } else if (mDir.z < -0.71) {
            return FACING::NEG_Z;
        }
        return FACING::POS_Y;
    }
    bool facingIsPos(FACING facing) {
        return facing == FACING::POS_X || facing == FACING::POS_Y || facing == FACING::POS_Z;
    }

    bool facingIsNeg(FACING facing) {
        return facing == FACING::NEG_X || facing == FACING::NEG_Y || facing == FACING::NEG_Z;
    }

    bool facingIsX(FACING facing) {
        return facing == FACING::POS_X || facing == FACING::NEG_X;
    }

    bool facingIsY(FACING facing) {
        return facing == FACING::POS_Y || facing == FACING::NEG_Y;
    }

    bool facingIsZ(FACING facing) {
        return facing == FACING::POS_Z || facing == FACING::NEG_Z;
    }
    BlockPos facingToPos(FACING facing, int length = 1) {
        switch (facing) {
            case FACING::POS_X:
                return BlockPos(length, 0, 0);
            case FACING::NEG_X:
                return BlockPos(-length, 0, 0);
            case FACING::NEG_Z:
                return BlockPos(0, 0, -length);
            case FACING::POS_Z:
                return BlockPos(0, 0, length);
            case FACING::NEG_Y:
                return BlockPos(0, -length, 0);
            case FACING::POS_Y:
                return BlockPos(0, length, 0);
            default:
                return BlockPos(0, 0, 0);
        }
    }

    FACING invFacing(FACING facing) {
        switch (facing) {
            case FACING::NEG_Y:
                return FACING::POS_Y;
            case FACING::POS_Y:
                return FACING::NEG_Y;
            case FACING::NEG_Z:
                return FACING::POS_Z;
            case FACING::POS_Z:
                return FACING::NEG_Z;
            case FACING::NEG_X:
                return FACING::POS_X;
            case FACING::POS_X:
                return FACING::NEG_X;
            default:
                return FACING::POS_X;
        }
    }

    void drawCurve(KochanekBartelsInterpolation const& curve, mce::ColorPalette color, int dimType) {
        auto length = (float)(curve.arcLength());
        length = std::min(length * 2.5f, MAX_LENGTH * 0.5f);
        int len = (int)ceil(length);
        auto particle = getParticleColorType(color);
        for (int i = 0; i <= len; ++i) {
            auto point = curve.getPosition(i / (double)len);
            globalPT().drawPoint(point, dimType, ParticleCUI::PointSize::PX4, mce::ColorPalette::YELLOW);
        }
    }
}  // namespace worldedit
