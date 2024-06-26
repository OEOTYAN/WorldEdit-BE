
#include "Clipboard.hpp"
#include "I18nAPI.h"
#include "WorldEdit.h"
#include "eval/Eval.h"

namespace we {

long long Clipboard::getIter(BlockPos const& pos) {
    return (pos.y + size.y * pos.z) * size.x + pos.x;
}
Clipboard::Clipboard(BlockPos const& sizes)
: size(sizes + 1),
  board(sizes),
  rotation(Rotation::None),
  mirror(Mirror::None),
  rotationAngle({0, 0, 0}) {
    used  = true;
    vsize = size.x * size.y * size.z;
    blockslist.clear();
    try {
        blockslist.resize(vsize);
    } catch (std::bad_alloc) {
        Level::broadcastText(tr("worldedit.memory.out"), TextType::RAW);
        return;
    }
}
Clipboard::Clipboard(const Clipboard& other) {
    if (other.entities) entities = other.entities->clone();
    size          = other.size;
    playerRelPos  = other.playerRelPos;
    playerPos     = other.playerPos;
    board         = other.board;
    rotation      = other.rotation;
    mirror        = other.mirror;
    rotationAngle = other.rotationAngle;
    flipY         = other.flipY;
    used          = other.used;
    vsize         = other.vsize;
    blockslist    = other.blockslist;
}
long long Clipboard::getIterLoop(BlockPos const& pos) {
    return (static_cast<int>(posfmod(pos.y, size.y))
            + size.y * static_cast<int>(posfmod(pos.z, size.z)))
             * size.x
         + static_cast<int>(posfmod(pos.x, size.x));
}
void Clipboard::storeBlock(BlockInstance& blockInstance, BlockPos const& pos) {
    if (pos.containedWithin(BlockPos(0, 0, 0), board)) {
        blockslist[getIter(pos)] = std::move(BlockNBTSet(blockInstance));
    }
}
BoundingBox Clipboard::getBoundingBox() {
    BoundingBox res;
    res.min = getPos({0, 0, 0});
    res.min = BlockPos::min(res.min, getPos({board.x, 0, 0}));
    res.min = BlockPos::min(res.min, getPos({0, board.y, 0}));
    res.min = BlockPos::min(res.min, getPos({0, 0, board.z}));
    res.min = BlockPos::min(res.min, getPos({board.x, 0, board.z}));
    res.min = BlockPos::min(res.min, getPos({board.x, board.y, 0}));
    res.min = BlockPos::min(res.min, getPos({0, board.y, board.z}));
    res.min = BlockPos::min(res.min, getPos({board.x, board.y, board.z}));
    res.max = getPos({0, 0, 0});
    res.max = BlockPos::max(res.max, getPos({board.x, 0, 0}));
    res.max = BlockPos::max(res.max, getPos({0, board.y, 0}));
    res.max = BlockPos::max(res.max, getPos({0, 0, board.z}));
    res.max = BlockPos::max(res.max, getPos({board.x, 0, board.z}));
    res.max = BlockPos::max(res.max, getPos({board.x, board.y, 0}));
    res.max = BlockPos::max(res.max, getPos({0, board.y, board.z}));
    res.max = BlockPos::max(res.max, getPos({board.x, board.y, board.z}));
    if (!(abs(posfmod(rotationAngle.x, 90.0f)) < 0.01
          && abs(posfmod(rotationAngle.y, 90.0f)) < 0.01
          && abs(posfmod(rotationAngle.z, 90.0f)) < 0.01)) {
        res.min = res.min - 1;
        res.max = res.max + 1;
    }
    return res;
}
void Clipboard::rotate(Vec3 angle) {
    rotationAngle   = rotationAngle + angle;
    rotationAngle.y = static_cast<float>(posfmod(rotationAngle.y, 360.0f));
    if (rotationAngle.y > 315 || rotationAngle.y <= 45) {
        rotation = Rotation::None;
    } else if (rotationAngle.y > 45 && rotationAngle.y <= 135) {
        rotation = Rotation::Rotate90;
    } else if (rotationAngle.y > 135 && rotationAngle.y <= 225) {
        rotation = Rotation::Rotate180;
    } else if (rotationAngle.y > 225 && rotationAngle.y <= 315) {
        rotation = Rotation::Rotate270;
    }
}
void Clipboard::flip(enum class FACING facing) {
    if (facing == FACING::NEG_Z || facing == FACING::POS_Z) {
        if (mirror == Mirror::None) {
            mirror = Mirror::X;
        } else if (mirror == Mirror::X) {
            mirror = Mirror::None;
        } else if (mirror == Mirror::Z) {
            mirror = Mirror::XZ;
        } else if (mirror == Mirror::XZ) {
            mirror = Mirror::Z;
        }
    } else if (facing == FACING::NEG_X || facing == FACING::POS_X) {
        if (mirror == Mirror::None) {
            mirror = Mirror::Z;
        } else if (mirror == Mirror::X) {
            mirror = Mirror::XZ;
        } else if (mirror == Mirror::Z) {
            mirror = Mirror::None;
        } else if (mirror == Mirror::XZ) {
            mirror = Mirror::X;
        }
    } else if (facing == FACING::NEG_Y || facing == FACING::POS_Y) {
        flipY = !flipY;
    }
}
BlockPos Clipboard::getPos(BlockPos const& pos) {
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
    angle.x    = static_cast<float>(posfmod(angle.x, 360.0f));
    angle.y    = static_cast<float>(posfmod(angle.y, 360.0f));
    angle.z    = static_cast<float>(posfmod(angle.z, 360.0f));

    if (abs(posfmod(angle.y, 90.0f)) < 0.01f) {
        if (abs(angle.y - 90) < 0.01f) {
            int tmp = res.x;
            res.x   = -res.z;
            res.z   = tmp;
        } else if (abs(angle.y - 180) < 0.01f) {
            res.x = -res.x;
            res.z = -res.z;
        } else if (abs(angle.y - 270) < 0.01f) {
            int tmp = res.x;
            res.x   = res.z;
            res.z   = -tmp;
        }
    } else {
        if (angle.y > 90 && angle.y < 270) {
            angle.y -= 180;
            res.x    = -res.x;
            res.z    = -res.z;
        }
        angle.y   = angle.y * static_cast<float>(M_PI / 180.0);
        auto tanY = tan(angle.y * 0.5);
        res.x     = static_cast<int>(floor(res.x + 0.5 - (res.z + 0.5) * tanY));
        res.z     = static_cast<int>(floor((res.x + 0.5) * sin(angle.y) + res.z + 0.5));
        res.x     = static_cast<int>(floor(res.x + 0.5 - (res.z + 0.5) * tanY));
    }

    if (abs(posfmod(angle.x, 90.0f)) < 0.01f) {
        if (abs(angle.x - 90) < 0.01f) {
            int tmp = res.y;
            res.y   = -res.z;
            res.z   = tmp;
        } else if (abs(angle.x - 180) < 0.01f) {
            res.y = -res.y;
            res.z = -res.z;
        } else if (abs(angle.x - 270) < 0.01f) {
            int tmp = res.y;
            res.y   = res.z;
            res.z   = -tmp;
        }
    } else {
        if (angle.x > 90 && angle.x < 270) {
            angle.x -= 180;
            res.y    = -res.y;
            res.z    = -res.z;
        }
        angle.x   = angle.x * static_cast<float>(M_PI / 180.0);
        auto tanX = tan(angle.x * 0.5);
        res.y     = static_cast<int>(floor(res.y + 0.5 - (res.z + 0.5) * tanX));
        res.z     = static_cast<int>(floor((res.y + 0.5) * sin(angle.x) + res.z + 0.5));
        res.y     = static_cast<int>(floor(res.y + 0.5 - (res.z + 0.5) * tanX));
    }
    if (abs(posfmod(angle.z, 90.0f)) < 0.01f) {
        if (abs(angle.z - 90) < 0.01f) {
            int tmp = res.x;
            res.x   = -res.y;
            res.y   = tmp;
        } else if (abs(angle.z - 180) < 0.01f) {
            res.x = -res.x;
            res.y = -res.y;
        } else if (abs(angle.z - 270) < 0.01f) {
            int tmp = res.x;
            res.x   = res.y;
            res.y   = -tmp;
        }
    } else {
        if (angle.z > 90 && angle.z < 270) {
            angle.z -= 180;
            res.x    = -res.x;
            res.y    = -res.y;
        }
        angle.z   = angle.z * static_cast<float>(M_PI / 180.0);
        auto tanZ = tan(angle.z * 0.5);
        res.x     = static_cast<int>(floor(res.x + 0.5 - (res.y + 0.5) * tanZ));
        res.y     = static_cast<int>(floor((res.x + 0.5) * sin(angle.z) + res.y + 0.5));
        res.x     = static_cast<int>(floor(res.x + 0.5 - (res.y + 0.5) * tanZ));
    }
    return res;
}
BlockNBTSet& Clipboard::getSet(BlockPos const& pos) { return blockslist[getIter(pos)]; }
BlockNBTSet& Clipboard::getSetLoop(BlockPos const& pos) {
    return blockslist[getIterLoop(pos)];
}
bool Clipboard::contains(BlockPos const& pos) {
    return blockslist[getIter(pos)].blocks.has_value();
}
bool Clipboard::setBlocks(
    BlockPos const&                                  pos,
    BlockPos&                                        worldPos,
    BlockSource*                                     blockSource,
    class PlayerData&                                data,
    class EvalFunctions&                             funcs,
    phmap::flat_hash_map<std::string, double> const& var,
    bool                                             setBiome
) {
    return blockslist[getIter(pos)]
        .setBlock(worldPos, blockSource, data, funcs, var, rotation, mirror, setBiome);
}
void Clipboard::forEachBlockInClipboard(const std::function<void(BlockPos const&)>& todo
) {
    for (int y = 0; y < size.y; y++)
        for (int x = 0; x < size.x; ++x)
            for (int z = 0; z < size.z; ++z) {
                if (contains({x, y, z})) {
                    todo({x, y, z});
                }
            }
}
} // namespace we