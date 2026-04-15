#pragma once

#include "pattern/SingleBlockPattern.h"

namespace we {
class HandPattern : public SingleBlockPattern {

public:
    static ll::Expected<std::shared_ptr<HandPattern>> create();

    ll::Expected<> prepare(CommandContextRef const& ctx) override;
};
} // namespace we
