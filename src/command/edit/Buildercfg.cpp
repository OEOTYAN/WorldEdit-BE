#include "builder/Builder.h"
#include "command/CommandMacro.h"

namespace we {
REG_CMD(edit, buildercfg, "Configure the builder type") {
    struct Params {
        BuilderType type;
    };
    command.overload<Params>().required("type").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, Params const& params) {
            auto lctx = getLocalContext(ctx);
            lctx->setupBuilder(params.type);
            ctx.success("builder type set to {0}", lctx->builder->getType());
        }
    );
}
} // namespace we
