#pragma once

#

namespace we {
class SimpleBuilder {
public:
    static long long buildCylinder(
        BlockPos    pos,
        int         dim,
        std::string xuid,
        class Pattern*,
        unsigned short radius = 0,
        int            height = 1,
        bool           hollow = false,
        std::string    mask   = ""
    );
    static long long buildSphere(
        BlockPos    pos,
        int         dim,
        std::string xuid,
        class Pattern*,
        unsigned short radius = 0,
        bool           hollow = false,
        std::string    mask   = ""
    );
    static long long buildCube(
        BlockPos    pos,
        int         dim,
        std::string xuid,
        class Pattern*,
        unsigned short size   = 0,
        bool           hollow = false,
        std::string    mask   = ""
    );
};
} // namespace we
