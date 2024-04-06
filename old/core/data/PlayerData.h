#pragma once
#include "brush/Brush.h"
#include "region/Region.h"
#include "store/Clipboard.hpp"
#include "store/History.h"
#include <mc/BedrockBlocks.hpp>
#include <mc/Block.hpp>
namespace we {
class PlayerData {
public:
    std::string xuid             = "";
    int         maxHistoryLength = 20;
    int         updateArg        = 0;
    int         updateExArg      = 1;

    BlockPos mainPos;
    int      mainPosTime = 0;
    int      mainPosDim  = -1;

    BlockPos vicePos;
    int      vicePosTime = 0;
    int      vicePosDim  = -1;

    std::string gMask = "";

    std::unique_ptr<class Region>                                   region = nullptr;
    class Clipboard                                                 clipboard;
    std::vector<class Clipboard>                                    historyList;
    int                                                             historyFlag1 = 0;
    int                                                             historyFlag2 = 0;
    phmap::flat_hash_map<std::string, std::unique_ptr<class Brush>> brushMap;

    PlayerData();
    PlayerData(class Player*);

    void                                                   clearHistory();
    class Clipboard&                                       getNextHistory();
    class Clipboard&                                       getCurrentHistory();
    std::optional<std::reference_wrapper<class Clipboard>> getUndoHistory();
    std::optional<std::reference_wrapper<class Clipboard>> getRedoHistory();
    bool changeMainPos(BlockInstance blockInstance, bool output = true);
    bool changeVicePos(BlockInstance blockInstance, bool output = true);
    void setVarByPlayer(phmap::flat_hash_map<::std::string, double>& variables);
    bool setBlockSimple(
        class BlockSource*                               blockSource,
        class EvalFunctions&                             funcs,
        phmap::flat_hash_map<std::string, double> const& var,
        BlockPos const&                                  pos,
        class Block const*                               block   = BedrockBlocks::mAir,
        class Block const*                               exblock = BedrockBlocks::mAir,
        std::optional<int> const&                        biomeId = std::nullopt
    );
    bool setBlockWithoutcheckGMask(
        class BlockSource*        blockSource,
        BlockPos const&           pos,
        class Block const*        block,
        class Block const*        exblock,
        std::optional<int> const& biomeId = std::nullopt
    );
};

} // namespace we