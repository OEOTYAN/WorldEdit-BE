//
// Created by OEOTYAN on 2022/05/15.
//
#pragma once
#ifndef WORLDEDIT_PARTICLE_H
#define WORLDEDIT_PARTICLE_H

// #include "pch.h"
// #include "WorldEdit.h"
// #include <string>
// #include "Graphics.h"
// #include "MC/Level.hpp"
namespace worldedit {

    enum class GRAPHIC_COLOR { WHITE = 0, RED = 1, YELLOW = 2, BLUE = 3, GREEN = 4 };

    void spawnParticle(Vec3 p, std::string& type, int dimType);

    void spawnCuboidParticle(const AABB& aabb, GRAPHIC_COLOR color, int dimType = 0);

}  // namespace worldedit
#endif  // WORLDEDIT_PARTICLE_H
