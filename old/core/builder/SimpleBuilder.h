//
// Created by OEOTYAN on 2022/06/09.
//
#pragma once

#

namespace worldedit {
    class SimpleBuilder {
       public:
        static long long buildCylinder(BlockPos pos,
                                       int dim,
                                       std::string xuid,
                                       class Pattern*,
                                       unsigned short radius = 0,
                                       int height = 1,
                                       bool hollow = false,
                                       std::string mask = "");
        static long long buildSphere(BlockPos pos,
                                     int dim,
                                     std::string xuid,
                                     class Pattern*,
                                     unsigned short radius = 0,
                                     bool hollow = false,
                                     std::string mask = "");
        static long long buildCube(BlockPos pos,
                                   int dim,
                                   std::string xuid,
                                   class Pattern*,
                                   unsigned short size = 0,
                                   bool hollow = false,
                                   std::string mask = "");
    };
}  // namespace worldedit
