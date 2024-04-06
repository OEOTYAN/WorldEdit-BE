#pragma once

#include "Globals.h"
#include "ParticleAPI.h"

namespace we {

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

FACING dirStringToFacing(std::string_view dir);

FACING dirToFacing(const Vec3& dir);

std::string facingToDirString(FACING facing);

void drawCurve(
    class KochanekBartelsInterpolation const& curve,
    enum mce::ColorPalette                    color,
    int                                       dimType
);

class ::ParticleCUI& globalPT();
} // namespace we
