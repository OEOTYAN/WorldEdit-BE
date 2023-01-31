//
// Created by OEOTYAN on 2022/05/30.
//
#pragma once
#ifndef WORLDEDIT_TOOL_H
#define WORLDEDIT_TOOL_H

#include "Brush.h"
#include <mc/BlockInstance.hpp>
#include <mc/Player.hpp>
#include <mc/Block.hpp>
#include <mc/BlockSource.hpp>
#include "store/BlockNBTSet.hpp"

namespace worldedit {
    class FarWand : public Brush {
       public:
        FarWand() = default;
        long long lset(Player* player, class BlockInstance blockInstance) override;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class AirWand : public Brush {
       public:
        AirWand() = default;
        long long lset(Player* player, class BlockInstance blockInstance) override;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class InfoTool : public Brush {
       public:
        InfoTool() = default;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class CyclerTool : public Brush {
       public:
        CyclerTool() = default;
        long long lset(Player* player, class BlockInstance blockInstance) override;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class FloodFillTool : public Brush {
       public:
        bool needEdge;
        FloodFillTool(BlockPattern* bp,int r,bool needEdge):Brush(r,bp),needEdge(needEdge){}
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class RepTool : public Brush {
       public:
       class BlockNBTSet blockSet;
       RepTool() = default;
       long long lset(Player* player, class BlockInstance blockInstance) override;
       long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class TreeTool : public Brush {
       public:
        TreeTool() = default;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class DelTreeTool : public Brush {
       public:
        DelTreeTool() = default;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };

}  // namespace worldedit

#endif  // WORLDEDIT_TOOL_H