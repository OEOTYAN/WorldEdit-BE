#include "command/CommandMacro.h"

namespace we {
REG_CMD(history, maxhistorylength, "Set the maximum history length for the player") {
    struct Params {
        int length;
    };
    command.overload<Params>().required("length").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, Params const& params) {
            auto lctx = getLocalContext(ctx);
            lctx->history.setMaxHistoryLength(params.length);
            ctx.success("max history length set to {0}", params.length);
        }
    );
}
} // namespace we
