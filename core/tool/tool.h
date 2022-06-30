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

namespace worldedit {
    class Tool {
       public:
        Tool() = default;
        virtual bool leftClick(Player* player, BlockInstance& blockInstance);
        virtual bool rightClick(Player* player, BlockInstance& blockInstance);
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
        FloodFillTool() = default;
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