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
    class FarWand final : public Brush {
       public:
        FarWand() = default;
        long long lset(Player* player, class BlockInstance blockInstance) override;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class AirWand final : public Brush {
       public:
        AirWand() = default;
        long long lset(Player* player, class BlockInstance blockInstance) override;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class InfoTool final : public Brush {
       public:
        InfoTool() = default;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class CyclerTool final : public Brush {
       public:
        CyclerTool() = default;
        long long lset(Player* player, class BlockInstance blockInstance) override;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class FloodFillTool final : public Brush {
       public:
        bool needEdge;
        FloodFillTool(std::unique_ptr<Pattern> bp, unsigned short r, bool needEdge) : Brush(r, std::move(bp)), needEdge(needEdge) {}
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class RepTool final : public Brush {
       public:
        class BlockNBTSet blockSet;
        RepTool() = default;
        long long lset(Player* player, class BlockInstance blockInstance) override;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class TreeTool final : public Brush {
       public:
        TreeTool() = default;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };
    class DelTreeTool final : public Brush {
       public:
        DelTreeTool() = default;
        long long set(Player* player, class BlockInstance blockInstance) override;
    };

}  // namespace worldedit

#endif  // WORLDEDIT_TOOL_H