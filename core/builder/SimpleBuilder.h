//
// Created by OEOTYAN on 2022/06/09.
//
#pragma once
#ifndef WORLDEDIT_SIMPLEBUILDER_H
#define WORLDEDIT_SIMPLEBUILDER_H

namespace worldedit {
    class SimpleBuilder {
       public:
        static long long buildCylinder(BlockPos    pos,
                                       int         dim,
                                       std::string xuid,
                                       class BlockPattern*,
                                       unsigned short radius = 0,
                                       int            height = 1,
                                       bool           hollow = false,
                                       std::string    mask   = "");
        static long long buildSphere(BlockPos    pos,
                                     int         dim,
                                     std::string xuid,
                                     class BlockPattern*,
                                     unsigned short radius = 0,
                                     bool           hollow = false,
                                     std::string    mask   = "");
        static long long buildCube(BlockPos    pos,
                                   int         dim,
                                   std::string xuid,
                                   class BlockPattern*,
                                   unsigned short size   = 0,
                                   bool           hollow = false,
                                   std::string    mask   = "");
    };
}  // namespace worldedit

#endif  // WORLDEDIT_SIMPLEBUILDER_H