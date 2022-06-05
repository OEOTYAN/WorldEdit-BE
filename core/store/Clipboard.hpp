//
// Created by OEOTYAN on 2022/05/21.
//
#pragma once
#ifndef WORLDEDIT_CLIPBOARD_H
#define WORLDEDIT_CLIPBOARD_H

// #include "pch.h"
// #include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockActor.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/CompoundTag.hpp>
// #include <MC/StructureSettings.hpp>
// #include <MC/VanillaBlockStateTransformUtils.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "string/StringTool.h"
// #include "particle/Graphics.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <DynamicCommandAPI.h>
// #include "store/BlockNBTSet.hpp"
// #include "WorldEdit.h"

#ifndef __M__PI__
#define __M__PI__ 3.141592653589793238462643383279
#endif

namespace worldedit {

    class Clipboard {
       public:
        BlockPos size;
        BlockPos playerRelPos;
        BlockPos playerPos;
        BlockPos board;
        Rotation rotation;
        Mirror mirror;
        Vec3 rotationAngle;
        bool flipY = false;
        bool used = false;
        long long vsize;
        std::vector<BlockNBTSet> blockslist;
        Clipboard() = default;
        Clipboard(const BlockPos& sizes)
            : size(sizes + BlockPos(1, 1, 1)),
              board(sizes),
              used(true),
              rotation(Rotation::None_14),
              mirror(Mirror::None_15),
              rotationAngle({0, 0, 0}) {
            vsize = size.x * size.y * size.z;
            blockslist.clear();
            blockslist.resize(vsize);
        }
        long long getIter(const BlockPos& pos) {
            return (pos.y + size.y * pos.z) * size.x + pos.x;
        }
        long long getIter2(const BlockPos& pos) {
            return (static_cast<int>(posfmod(pos.y, size.y)) +
                    size.y * static_cast<int>(posfmod(pos.z, size.z))) *
                       size.x +
                   static_cast<int>(posfmod(pos.x, size.x));
        }
        void storeBlock(BlockInstance& blockInstance, const BlockPos& pos) {
            if (pos.containedWithin(BlockPos(0, 0, 0), board)) {
                blockslist[getIter(pos)] = BlockNBTSet(blockInstance);
            }
        }
        BoundingBox getBoundingBox() {
            BoundingBox res;
            res.bpos1 = getPos({0, 0, 0});
            res.bpos1 = min(res.bpos1, getPos({board.x, 0, 0}));
            res.bpos1 = min(res.bpos1, getPos({0, board.y, 0}));
            res.bpos1 = min(res.bpos1, getPos({0, 0, board.z}));
            res.bpos1 = min(res.bpos1, getPos({board.x, 0, board.z}));
            res.bpos1 = min(res.bpos1, getPos({board.x, board.y, 0}));
            res.bpos1 = min(res.bpos1, getPos({0, board.y, board.z}));
            res.bpos1 = min(res.bpos1, getPos({board.x, board.y, board.z}));
            res.bpos2 = getPos({0, 0, 0});
            res.bpos2 = max(res.bpos2, getPos({board.x, 0, 0}));
            res.bpos2 = max(res.bpos2, getPos({0, board.y, 0}));
            res.bpos2 = max(res.bpos2, getPos({0, 0, board.z}));
            res.bpos2 = max(res.bpos2, getPos({board.x, 0, board.z}));
            res.bpos2 = max(res.bpos2, getPos({board.x, board.y, 0}));
            res.bpos2 = max(res.bpos2, getPos({0, board.y, board.z}));
            res.bpos2 = max(res.bpos2, getPos({board.x, board.y, board.z}));
            if (!(abs(posfmod(rotationAngle.x, 90.0f)) < 0.01 &&
                  abs(posfmod(rotationAngle.y, 90.0f)) < 0.01 &&
                  abs(posfmod(rotationAngle.z, 90.0f)) < 0.01)) {
                res.bpos1 = res.bpos1 - 1;
                res.bpos2 = res.bpos2 + 1;
            }
            return res;
        }
        void rotate(Vec3 angle) {
            rotationAngle = rotationAngle + angle;
            rotationAngle.y =
                static_cast<float>(posfmod(rotationAngle.y, 360.0f));
            if (rotationAngle.y > 315 || rotationAngle.y <= 45) {
                rotation = Rotation::None_14;
            } else if (rotationAngle.y > 45 && rotationAngle.y <= 135) {
                rotation = Rotation::Rotate90;
            } else if (rotationAngle.y > 135 && rotationAngle.y <= 225) {
                rotation = Rotation::Rotate180;
            } else if (rotationAngle.y > 225 && rotationAngle.y <= 315) {
                rotation = Rotation::Rotate270;
            }
        }
        void flip(FACING facing) {
            if (facing == FACING::NEG_Z || facing == FACING::POS_Z) {
                if (mirror == Mirror::None_15) {
                    mirror = Mirror::X;
                } else if (mirror == Mirror::X) {
                    mirror = Mirror::None_15;
                } else if (mirror == Mirror::Z) {
                    mirror = Mirror::XZ;
                } else if (mirror == Mirror::XZ) {
                    mirror = Mirror::Z;
                }
            } else if (facing == FACING::NEG_X || facing == FACING::POS_X) {
                if (mirror == Mirror::None_15) {
                    mirror = Mirror::Z;
                } else if (mirror == Mirror::X) {
                    mirror = Mirror::XZ;
                } else if (mirror == Mirror::Z) {
                    mirror = Mirror::None_15;
                } else if (mirror == Mirror::XZ) {
                    mirror = Mirror::X;
                }
            } else if (facing == FACING::NEG_Y || facing == FACING::POS_Y) {
                flipY = !flipY;
            }
        }
        BlockPos getPos(const BlockPos& pos) {
            BlockPos res = pos - playerRelPos;
            if (mirror == Mirror::XZ) {
                res.x = -res.x;
                res.z = -res.z;
            } else if (mirror == Mirror::X) {
                res.z = -res.z;
            } else if (mirror == Mirror::Z) {
                res.x = -res.x;
            }
            if (flipY) {
                res.y = -res.y;
            }
            Vec3 angle = rotationAngle;
            angle.x = static_cast<float>(posfmod(angle.x, 360.0f));
            angle.y = static_cast<float>(posfmod(angle.y, 360.0f));
            angle.z = static_cast<float>(posfmod(angle.z, 360.0f));

            if (abs(posfmod(angle.y, 90.0f)) < 0.01f) {
                if (abs(angle.y - 90) < 0.01f) {
                    int tmp = res.x;
                    res.x = -res.z;
                    res.z = tmp;
                } else if (abs(angle.y - 180) < 0.01f) {
                    res.x = -res.x;
                    res.z = -res.z;
                } else if (abs(angle.y - 270) < 0.01f) {
                    int tmp = res.x;
                    res.x = res.z;
                    res.z = -tmp;
                }
            } else {
                if (angle.y > 90 && angle.y < 270) {
                    angle.y -= 180;
                    res.x = -res.x;
                    res.z = -res.z;
                }
                angle.y = angle.y * static_cast<float>(__M__PI__ / 180.0);
                auto tanY = tan(angle.y * 0.5);
                res.x =
                    static_cast<int>(floor(res.x + 0.5 - (res.z + 0.5) * tanY));
                res.z = static_cast<int>(
                    floor((res.x + 0.5) * sin(angle.y) + res.z + 0.5));
                res.x =
                    static_cast<int>(floor(res.x + 0.5 - (res.z + 0.5) * tanY));
            }

            if (abs(posfmod(angle.x, 90.0f)) < 0.01f) {
                if (abs(angle.x - 90) < 0.01f) {
                    int tmp = res.y;
                    res.y = -res.z;
                    res.z = tmp;
                } else if (abs(angle.x - 180) < 0.01f) {
                    res.y = -res.y;
                    res.z = -res.z;
                } else if (abs(angle.x - 270) < 0.01f) {
                    int tmp = res.y;
                    res.y = res.z;
                    res.z = -tmp;
                }
            } else {
                if (angle.x > 90 && angle.x < 270) {
                    angle.x -= 180;
                    res.y = -res.y;
                    res.z = -res.z;
                }
                angle.x = angle.x * static_cast<float>(__M__PI__ / 180.0);
                auto tanX = tan(angle.x * 0.5);
                res.y =
                    static_cast<int>(floor(res.y + 0.5 - (res.z + 0.5) * tanX));
                res.z = static_cast<int>(
                    floor((res.y + 0.5) * sin(angle.x) + res.z + 0.5));
                res.y =
                    static_cast<int>(floor(res.y + 0.5 - (res.z + 0.5) * tanX));
            }
            if (abs(posfmod(angle.z, 90.0f)) < 0.01f) {
                if (abs(angle.z - 90) < 0.01f) {
                    int tmp = res.x;
                    res.x = -res.y;
                    res.y = tmp;
                } else if (abs(angle.z - 180) < 0.01f) {
                    res.x = -res.x;
                    res.y = -res.y;
                } else if (abs(angle.z - 270) < 0.01f) {
                    int tmp = res.x;
                    res.x = res.y;
                    res.y = -tmp;
                }
            } else {
                if (angle.z > 90 && angle.z < 270) {
                    angle.z -= 180;
                    res.x = -res.x;
                    res.y = -res.y;
                }
                angle.z = angle.z * static_cast<float>(__M__PI__ / 180.0);
                auto tanZ = tan(angle.z * 0.5);
                res.x =
                    static_cast<int>(floor(res.x + 0.5 - (res.y + 0.5) * tanZ));
                res.y = static_cast<int>(
                    floor((res.x + 0.5) * sin(angle.z) + res.y + 0.5));
                res.x =
                    static_cast<int>(floor(res.x + 0.5 - (res.y + 0.5) * tanZ));
            }
            return res;
        }
        BlockNBTSet getSet(const BlockPos& pos) {
            return blockslist[getIter(pos)];
        }
        BlockNBTSet getSet2(const BlockPos& pos) {
            return blockslist[getIter2(pos)];
        }
        bool contains(const BlockPos& pos) {
            return blockslist[getIter(pos)].hasBlock;
        }
        void setBlocks(const BlockPos& pos,
                       BlockPos& worldPos,
                       BlockSource* blockSource) {
            blockslist[getIter(pos)].setBlock(worldPos, blockSource, rotation,
                                               mirror);
        }
        void forEachBlockInClipboard(
            const std::function<void(const BlockPos&)>& todo) {
            for (int y = 0; y < size.y; y++)
                for (int x = 0; x < size.x; x++)
                    for (int z = 0; z < size.z; z++) {
                        if (contains({x, y, z})) {
                            todo({x, y, z});
                        }
                    }
        }
    };

}  // namespace worldedit

#endif  // WORLDEDIT_CLIPBOARD_H