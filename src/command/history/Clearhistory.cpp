#include "command/CommandMacro.h"

namespace we {
REG_CMD(history, clearhistory, "Clear the history for the player") {
    command.overload().execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx) {
            auto lctx = getLocalContext(ctx);
            auto size = lctx->history.size();
            lctx->history.clear();
            ctx.success("cleared {0} history records", size);
        }
    );
}
} // namespace we