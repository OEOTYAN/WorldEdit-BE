#pragma once

#include "worldedit/Global.h"

namespace we {

void setupCommands();

bool addSetup(std::string name, std::function<void()>) noexcept;

} // namespace we
