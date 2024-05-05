#pragma once

namespace we {
void setupSel();
void setupLoftcycle();
void setupPos1();
void setupPos2();
void setupHpos1();
void setupHpos2();
void setupChunk();

void setupRegionCommands() {
    setupSel();
    setupLoftcycle();
    setupPos1();
    setupPos2();
    setupHpos1();
    setupHpos2();
    setupChunk();
}
} // namespace we
