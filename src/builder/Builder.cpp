#include "Builder.h"

#include "InplaceBuilder.h"
#include "SimulatedPlayerBuilder.h"

namespace we {
std::shared_ptr<Builder> Builder::create(BuilderType type, LocalContext& context) {
    std::shared_ptr<Builder> result;
    switch (type) {
    case BuilderType::Inplace:
        result = std::make_shared<InplaceBuilder>(context);
        break;
    case BuilderType::InplaceNoNc:
        result = std::make_shared<InplaceBuilder>(context, BlockUpdateFlag::Network);
        break;
    case BuilderType::Bot:
        result = std::make_shared<SimulatedPlayerBuilder>(context);
        break;
    case BuilderType::None:
    default:
        result = std::make_shared<Builder>(context);
    }
    result->type = type;
    result->setup();
    return result;
}
} // namespace we