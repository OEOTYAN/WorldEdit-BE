#include "Builder.h"

#include "InplaceBuilder.h"
#include "SimulatedPlayerBuilder.h"

namespace we {
std::unique_ptr<Builder> Builder::create(BuilderType type, LocalContext& context) {
    std::unique_ptr<Builder> result;
    switch (type) {
    case BuilderType::Inplace:
        result = std::make_unique<InplaceBuilder>(context);
        break;
    case BuilderType::InplaceNoNc:
        result = std::make_unique<InplaceBuilder>(context, BlockUpdateFlag::Network);
        break;
    case BuilderType::Bot:
        result = std::make_unique<SimulatedPlayerBuilder>(context);
        break;
    case BuilderType::None:
    default:
        result = std::make_unique<Builder>(context);
    }
    result->type = type;
    return result;
}
} // namespace we