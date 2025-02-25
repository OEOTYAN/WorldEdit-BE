#pragma once

#include <expected>

#include "utils/Hash.h"
#include "utils/InplaceVector.h"
#include "utils/WithDim.h"
#include <bsci/GeometryGroup.h>
#include <ll/api/Expected.h>
#include <ll/api/Global.h>
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/command/runtime/ParamKind.h>
#include <ll/api/coro/CoroTask.h>
#include <ll/api/coro/Generator.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/Logger.h>
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ServerThreadExecutor.h>
#include <ll/api/thread/ThreadPoolExecutor.h>
#include <ll/api/utils/ErrorUtils.h>
#include <ll/api/utils/HashUtils.h>
#include <ll/api/utils/StringUtils.h>
#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/item/SaveContext.h>
#include <mc/world/item/VanillaItemNames.h>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/ChunkBlockPos.h>
#include <mc/world/level/ChunkPos.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/Pos2d.h>
#include <mc/world/level/SubChunkPos.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/dimension/DimensionHeightRange.h>
#include <mc/world/level/levelgen/structure/BoundingBox.h>
#include <mc/world/phys/AABB.h>
#include <mc/world/phys/HitResult.h>

using namespace ll::literals;

namespace we {
class WorldEdit;
}
