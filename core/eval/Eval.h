#pragma once
//
// Created by OEOTYAN on 2022/5/15.
//
#ifndef WORLDEDIT_EVAL_H
#define WORLDEDIT_EVAL_H

#include "Global.h"
#include <MC/Biome.hpp>
// #include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BedrockBlocks.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Level.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
#include "CppEval.h"
#include "FastNoiseLite.h"

#ifndef __M__PI__
#define __M__PI__ 3.141592653589793238462643383279
#endif

namespace worldedit {

    double posfmod(double x, double y);

    int getHighestTerrainBlock(class BlockSource* blockSource, int x, int z, int minY, int maxY, std::string filter);

    class LongLong3 {
       public:
        long long x = 0;
        long long y = 0;
        long long z = 0;

        constexpr LongLong3& operator+=(LongLong3 const& b) {
            x += b.x;
            y += b.y;
            z += b.z;
            return *this;
        }

        constexpr LongLong3& operator-=(LongLong3 const& b) {
            x -= b.x;
            y -= b.y;
            z -= b.z;
            return *this;
        }
    };
    class EvalFunctions {
        int normalSearchDis = 4;
        BlockPos here;
        BlockPos size;
        BlockSource* blockSource;
        BoundingBox normalSearchBox;
        std::vector<LongLong3> posMap;
        std::vector<long long> solidMap;
        bool searchBoxInitialized = false;
        bool blockdataInitialized = false;

       public:
        void setPos(const BlockPos& pos) { here = pos; }
        void setbs(BlockSource* bs) {
            blockSource = bs;
            blockdataInitialized = true;
        }
        void setbox(BoundingBox box);
        long long getIndex(const BlockPos& pos);
        long long getSolidMap(const BlockPos& pos1, const BlockPos& pos2);
        LongLong3 getPosMap(const BlockPos& pos1, const BlockPos& pos2);
        double operator()(std::string const& name, const std::vector<double>& params);
    };
}  // namespace worldedit
#endif