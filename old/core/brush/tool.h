#pragma once
#ifndef WORLDEDIT_TOOL_H
#define WORLDEDIT_TOOL_H

#include "Brush.h"
#include "store/BlockNBTSet.hpp"
#include <mc/Block.hpp>
#include <mc/BlockInstance.hpp>
#include <mc/BlockSource.hpp>
#include <mc/Player.hpp>

namespace we {
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
    FloodFillTool(std::unique_ptr<Pattern> bp, unsigned short r, bool needEdge)
    : Brush(r, std::move(bp)),
      needEdge(needEdge) {}
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

} // namespace we

#endif // WORLDEDIT_TOOL_H