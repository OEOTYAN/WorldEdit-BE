#pragma once

namespace we {
void setupSel();
void setupLoftcycle();
void setupPos();

void setupRegionCommands() {
    setupSel();
    setupLoftcycle();
    setupPos();
}
} // namespace we
