//
// Created by OEOTYAN on 2022/5/15.
//
#pragma once
#ifndef WORLDEDIT_GRAPHICS_H
#define WORLDEDIT_GRAPHICS_H

#include "pch.h"
#include "Particle.h"
// #include "string/StringTool.h"
// #include "WorldEdit.h"

namespace worldedit {

    enum class DIRECTION {
        NEG_Y = 0,
        POS_Y = 1,
        NEG_Z = 2,
        POS_Z = 3,
        NEG_X = 4,
        POS_X = 5,
    };
    enum class FACING {
        NEG_Y = 0,
        POS_Y = 1,
        NEG_Z = 2,
        POS_Z = 3,
        NEG_X = 4,
        POS_X = 5,
    };

    bool facingIsPos(FACING facing);

    bool facingIsNeg(FACING facing);

    bool facingIsX(FACING facing);

    bool facingIsY(FACING facing);

    bool facingIsZ(FACING facing);

    FACING invFacing(FACING facing);

    std::string facingToString(FACING facing);

    BlockPos facingToPos(FACING facing, int length);

    FACING dirStringToFacing(const std::string& dir);

    FACING dirToFacing(const Vec3& dir);

    std::string facingToDirString(FACING facing);

    void drawLine(const Vec3& originPoint, FACING direction, float length, GRAPHIC_COLOR color, int dimType);
    void drawOrientedLine(const Vec3& start, const Vec3& end, int dimType, GRAPHIC_COLOR color);
    void drawCircle(const Vec3& originPoint, FACING direction, float radius, GRAPHIC_COLOR color, int dimType);
    void drawCurve(class KochanekBartelsInterpolation const& curve, GRAPHIC_COLOR color, int dimType);
}  // namespace worldedit

#endif  // WORLDEDIT_GRAPHICS_H
