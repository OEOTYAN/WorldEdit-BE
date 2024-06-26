
#include "SphereBrush.h"
#include "builder/SimpleBuilder.h"
#include "store/Patterns.h"
namespace we {
SphereBrush::SphereBrush(unsigned short s, std::unique_ptr<Pattern> bp, bool a)
: Brush(s, std::move(bp)),
  hollow(a) {}
long long SphereBrush::set(Player* player, BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }
    return SimpleBuilder::buildSphere(
        blockInstance.getPosition(),
        blockInstance.getDimensionId(),
        player->getXuid(),
        pattern.get(),
        size,
        hollow,
        mask
    );
}
} // namespace we