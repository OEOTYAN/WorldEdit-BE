//
// Created by OEOTYAN on 2022/05/30.
//
#pragma once
#ifndef WORLDEDIT_TOOL_H
#define WORLDEDIT_TOOL_H

#include <MC/BlockInstance.hpp>
#include <MC/Player.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include "store/BlockNBTSet.hpp"

namespace worldedit {
    class Tool {
       public:
        bool needFace = false;
        bool lneedFace = false;
        Tool() = default;
        virtual bool leftClick(Player* player, BlockInstance& blockInstance);
        virtual bool rightClick(Player* player, BlockInstance& blockInstance);
        virtual ~Tool() = default;
    };
    class FarWand : public Tool {
       public:
        FarWand() = default;
        bool leftClick(Player* player, BlockInstance& blockInstance) override;
        bool rightClick(Player* player, BlockInstance& blockInstance) override;
    };
    class AirWand : public Tool {
       public:
        AirWand() = default;
        bool leftClick(Player* player, BlockInstance& blockInstance) override;
        bool rightClick(Player* player, BlockInstance& blockInstance) override;
    };
    class InfoTool : public Tool {
       public:
        InfoTool() = default;
        bool rightClick(Player* player, BlockInstance& blockInstance) override;
    };
    class CyclerTool : public Tool {
       public:
        CyclerTool() = default;
        bool leftClick(Player* player, BlockInstance& blockInstance) override;
        bool rightClick(Player* player, BlockInstance& blockInstance) override;
    };
    class FloodFillTool : public Tool {
       public:
        class BlockPattern* pattern;
        int radius;
        bool needEdge;
        FloodFillTool(BlockPattern* bp,int r,bool n):pattern(bp),radius(r),needEdge(n){}
        bool rightClick(Player* player, BlockInstance& blockInstance) override;
        ~FloodFillTool();
    };
    class RepTool : public Tool {
       public:
       class BlockNBTSet blockSet;
       RepTool() = default;
       bool leftClick(Player* player, BlockInstance& blockInstance) override;
       bool rightClick(Player* player, BlockInstance& blockInstance) override;
    };
    class TreeTool : public Tool {
       public:
        TreeTool() = default;
        bool rightClick(Player* player, BlockInstance& blockInstance) override;
    };
    class DelTreeTool : public Tool {
       public:
        DelTreeTool() = default;
        bool rightClick(Player* player, BlockInstance& blockInstance) override;
    };

}  // namespace worldedit

#endif  // WORLDEDIT_TOOL_H