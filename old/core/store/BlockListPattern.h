
#pragma once

#include "Pattern.h"

namespace we {
class BlockNameType {
public:
    std::string val;
    BlockNameType(std::string const& v) : val(v) {}
};
class Percents {
public:
    std::variant<double, std::string> val = 1.0;
    Percents()                            = default;

    double getPercents(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs
    );
};
class RawBlock {
public:
    using blockid_t = std::pair<
        std::variant<std::string, BlockNameType>,
        std::optional<std::variant<int, std::string>>>;
    std::variant<class Block const*, std::string, blockid_t> block;
    class Block const*                                       exBlock;
    std::unique_ptr<CompoundTag>                             blockEntity = nullptr;
    RawBlock();

    class Block const* getBlock(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs
    );
};

class BlockListPattern : public Pattern {
public:
    size_t                blockNum = 0;
    std::vector<Percents> percents;
    std::vector<RawBlock> rawBlocks;

    BlockListPattern(std::string_view str, std::string_view xuid);

    class Block const* getBlock(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs
    ) override;

    bool hasBlock(class Block const* block) override;

    bool setBlock(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs,
        BlockSource*                                       blockSource,
        BlockPos const&                                    pos
    ) override;

private:
    class RawBlock* getRawBlock(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs
    );
};

} // namespace we