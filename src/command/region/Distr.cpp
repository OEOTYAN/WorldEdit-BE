#include "command/CommandMacro.h"
#include <mc/world/Container.h>
#include <mc/world/item/ItemStack.h>
#include "utils/BlockUtils.h"
#include <mc/world/level/block/Block.h>
#include <numeric>

namespace we {
struct DistrMatchParams {
    CommandBlockName block{};
    struct VaArgs {
        bool detailed{false};
        bool container_included{false};
    } args{};
};

static std::string_view
remove_prefix(std::string_view name, std::string_view prefix = "minecraft:") {
    if (name.starts_with(prefix)) {
        name.remove_prefix(prefix.size());
    }
    return name;
}

static std::string format_block_name(Block const& block, bool detailed) {
    auto name = std::string(remove_prefix(block.getTypeName()));
    if (!detailed || !block.mSerializationId->contains("states", Tag::Type::Compound)) {
        return name;
    }
    CompoundTag states;
    for (auto& [key, value] : block.mSerializationId->at("states").items()) {
        states[remove_prefix(key)] = value;
    }
    if (states.empty()) {
        return name;
    }
    auto statesStr = states.toSnbt(SnbtFormat::Minimize, 0);
    return fmt::format(
        "{}[{}]",
        name,
        std::string_view{statesStr}.substr(1, statesStr.size() - 2)
    );
}

static std::string format_item_name(ItemStack const& item, bool detailed) {
    std::string name;
    if (item.mBlock) {
        name = format_block_name(*item.mBlock, detailed);
    } else {
        name = remove_prefix(item.getTypeName());
    }
    if (!detailed) {
        return name;
    }
    auto customName = item.getCustomName();
    if (customName.empty()) {
        return name;
    }
    return fmt::format("{}[name=\"{}\"]", name, customName);
}

class DistrCounter {
    ll::SmallDenseMap<std::string, uint64> unsortedDistr;
    size_t                                 total = 0;

public:
    void tick(uint64 count = 1) { total += count; }
    void submit(std::string name, uint64 count = 1) {
        unsortedDistr[std::move(name)] += count;
    }
    void report(CommandContextRef const& ctx) {
        if (unsortedDistr.empty() || total == 0) {
            ctx.success("Empty data");
            return;
        }
        ctx.success("Total: {}", total);
        std::vector<std::pair<std::string, uint64>> distr{
            unsortedDistr.begin(),
            unsortedDistr.end()
        };
        std::sort(distr.begin(), distr.end(), [](auto const& a, auto const& b) {
            return a.second > b.second;
        });
        for (auto const& [name, count] : distr) {
            auto ratio = static_cast<double>(count) / total * 100.0;
            ctx.success("{} ({:.3f}%%) {}", count, ratio, name);
        }
    }
};

static void count_container(
    Container&                                  container,
    DistrCounter&                               counter,
    bool                                        detailed,
    brstd::function_ref<bool(ItemStack const&)> itemFilter
) {
    std::queue<std::unique_ptr<CompoundTag>> itemQueue;
    for (auto& item : container) {
        if (item.mCount > 0) {
            itemQueue.emplace(item.save({}));
        }
    }

    while (!itemQueue.empty()) {
        auto nbt  = std::move(itemQueue.front());
        auto item = ItemStack::fromTag(*nbt);
        itemQueue.pop();
        auto count = item.mCount;
        if (count > 0 && itemFilter(item)) {
            counter.tick(count);
            counter.submit(format_item_name(item, detailed), count);
        }
        if (nbt->contains("tag", Tag::Type::Compound)
            && nbt->at("tag").contains("Items", Tag::Type::List)) {
            for (auto&& sub : nbt->at("tag")["Items"].get<ListTag>()) {
                if (sub.is_object())
                    itemQueue.emplace(
                        std::make_unique<CompoundTag>(sub.get<CompoundTag>())
                    );
            }
        }
    }
}

REG_CMD(region, distr, "show region block distribution or count matched blocks") {
    auto listExecutor = [](CommandContextRef const&                    ctx,
                           DistrMatchParams::VaArgs const&             args,
                           brstd::function_ref<bool(Block const&)>     blockFilter,
                           brstd::function_ref<bool(ItemStack const&)> itemFilter) {
        auto region = checkRegion(ctx);
        if (!region) return;
        auto dim = checkDimension(ctx);
        if (!dim) return;
        if (region->getDim() != dim->getDimensionId()) {
            ctx.error("origin dimension doesn't match region dimension");
            return;
        }
        auto& source = dim->getBlockSourceFromMainChunkSource();

        DistrCounter counter;
        region->forEachBlockInRegion([&](BlockPos const& pos) {
            auto const& block      = source.getBlock(pos);
            auto const& extraBlock = source.getExtraBlock(pos);

            if (blockFilter(block)) {
                counter.tick();
                if (extraBlock.isAir()) {
                    counter.submit(format_block_name(block, args.detailed));
                } else {
                    counter.submit(
                        fmt::format(
                            "{} with {}",
                            format_block_name(block, args.detailed),
                            format_block_name(extraBlock, args.detailed)
                        )
                    );
                }
            }

            if (!args.container_included) {
                return;
            }

            if (auto container = source.tryGetContainer(pos)) {
                count_container(*container, counter, args.detailed, itemFilter);
            }
        });
        counter.report(ctx);
    };

    command.overload<DistrMatchParams>().optional("args").execute(
        CmdCtxBuilder{} |
        [listExecutor](CommandContextRef const& ctx, DistrMatchParams const& params) {
            listExecutor(
                ctx,
                params.args,
                [](auto&&) { return true; },
                [](auto&&) { return true; }
            );
        }
    );

    command.overload<DistrMatchParams>().required("block").optional("args").execute(
        CmdCtxBuilder{} |
        [listExecutor](CommandContextRef const& ctx, DistrMatchParams const& params) {
            HashedString blockName = HashedString{params.block.getBlockName()};
            auto         block     = Block::tryGetFromRegistry(blockName);
            if (!block) {
                ctx.error("invalid block name");
                return;
            }
            listExecutor(
                ctx,
                params.args,
                [&](Block const& blockRef) {
                    return blockRef.getBlockType().mNameInfo->mFullName->getHash()
                        == blockName.getHash();
                },
                [&](ItemStack const& item) {
                    if (item.mBlock) {
                        return item.mBlock->getBlockType().mNameInfo->mFullName->getHash()
                            == blockName.getHash();
                    }
                    return false;
                }
            );
        }
    );
}
} // namespace we
