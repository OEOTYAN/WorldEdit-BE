#include "command/Commands.h"
#include "data/PlayerStateManager.h"
#include "region/Region.h"
#include "worldedit/WorldEdit.h"

namespace we {

struct Sel {
    RegionType type;
};
enum class SelRemoveType {
    Last,
    Near,
};
struct SelRm {
    SelRemoveType type;
};

static bool _ = addSetup("sel", [] {
    auto& config = WorldEdit::getInstance().getConfig().commands.region.sel;
    if (!config.enabled) {
        return;
    }
    auto& command = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "sel",
        "manipulate region"_tr(),
        config.permission
    );

    command.overload().text("clear").execute([](CommandOrigin const& origin,
                                                CommandOutput&       output) {
        auto state = WorldEdit::getInstance().getPlayerStateManager().get(origin);
        if (!state) {
            output.error("origin didn't have state"_tr());
            return;
        }
        state->region.reset();
        state->mainPos.reset();
        state->offPos.reset();
        output.success("region cleared"_tr());
    });
    command.overload<SelRm>().text("remove").required("type").execute(
        [](CommandOrigin const& origin, CommandOutput& output, SelRm const& params) {
            auto state = WorldEdit::getInstance().getPlayerStateManager().get(origin);
            if (!state) {
                output.error("origin didn't have state"_tr());
                return;
            }
            if (!state->region) {
                output.error("origin didn't selected any region"_tr());
                return;
            }
            if (auto* dim = origin.getDimension();
                !dim || dim->getDimensionId() != state->region->getDim()) {
                output.error("origin dimension doesn't match region dimension"_tr());
                return;
            }
            bool res;
            if (params.type == SelRemoveType::Last) {
                res = state->region->removePoint(std::nullopt);
            } else {
                res = state->region->removePoint(origin.getBlockPosition());
            }
            if (res) {
                output.success("selected postion removed"_tr());
            } else {
                output.error("fail to remove selected postion"_tr());
            }
        }
    );

    command.overload<Sel>().required("type").execute([](CommandOrigin const& origin,
                                                        CommandOutput&       output,
                                                        Sel const&           params) {
        auto state = WorldEdit::getInstance().getPlayerStateManager().getOrCreate(origin);
        if (state->region) {
            state->region = Region::create(
                params.type,
                state->region->getDim(),
                state->region->getBoundBox()
            );
        } else {
            state->region = nullptr;
        }
        state->regionType = params.type;
        state->mainPos.reset();
        state->offPos.reset();
        output.success("region switch to {0}"_tr(params.type));
    });
});
} // namespace we
