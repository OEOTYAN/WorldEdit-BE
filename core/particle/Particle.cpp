//
// Created by OEOTYAN on 2022/5/15.
//
#pragma once
#include "pch.h"
// #include "WorldEdit.h"
#include "Particle.h"
#include "Graphics.h"
// #include <string>
#include "MC/Player.hpp"
#include "MC/Dimension.hpp"
#include "MC/Level.hpp"

namespace worldedit {

    void spawnParticle(Vec3 const& p, std::string const& type, int dimType) {
        auto diminsion = Global<Level>->getDimension(dimType);
        diminsion->forEachPlayer([&](Player& player) {
            player.sendSpawnParticleEffectPacket(p, dimType, type);
            return true;
        });
    }

    void spawnCuboidParticle(const AABB& aabb, GRAPHIC_COLOR color, int dimType) {
        auto p1 = aabb.pointA, p2 = aabb.pointB;
        auto dx = p2.x - p1.x;
        auto dy = p2.y - p1.y;
        auto dz = p2.z - p1.z;
        drawLine(p1, FACING::POS_X, dx, color, dimType);
        drawLine(p1, FACING::POS_Y, dy, color, dimType);
        drawLine(p1, FACING::POS_Z, dz, color, dimType);
        Vec3 p3{p2.x, p1.y, p2.z};
        drawLine(p3, FACING::NEG_X, dx, color, dimType);
        drawLine(p3, FACING::POS_Y, dy, color, dimType);
        drawLine(p3, FACING::NEG_Z, dz, color, dimType);
        Vec3 p4{p2.x, p2.y, p1.z};
        drawLine(p4, FACING::NEG_X, dx, color, dimType);
        drawLine(p4, FACING::NEG_Y, dy, color, dimType);
        drawLine(p4, FACING::POS_Z, dz, color, dimType);
        Vec3 p5{p1.x, p2.y, p2.z};
        drawLine(p5, FACING::POS_X, dx, color, dimType);
        drawLine(p5, FACING::NEG_Y, dy, color, dimType);
        drawLine(p5, FACING::NEG_Z, dz, color, dimType);
    }
}  // namespace worldedit