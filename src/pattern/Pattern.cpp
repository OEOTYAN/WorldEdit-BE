#include "pattern/Pattern.h"
#include "pattern/BlockListPattern.h"
#include "pattern/ClipboardPattern.h"
#include "pattern/GradientPattern.h"
#include "pattern/HandPattern.h"

namespace we {
ll::Expected<std::shared_ptr<Pattern>> Pattern::fromAst(PatternAst const& ast) {
    return std::visit(
        []<class T>(T const& node) -> ll::Expected<std::shared_ptr<Pattern>> {
            if constexpr (std::is_same_v<T, HandPatternAst>) {
                auto pattern = HandPattern::create();
                if (!pattern) {
                    return ll::forwardError(pattern.error());
                }
                return std::static_pointer_cast<Pattern>(*pattern);
            }
            if constexpr (std::is_same_v<T, ClipboardPatternAst>) {
                auto pattern = ClipboardPattern::create(node);
                if (!pattern) {
                    return ll::forwardError(pattern.error());
                }
                return std::static_pointer_cast<Pattern>(*pattern);
            }
            if constexpr (std::is_same_v<T, GradientPatternAst>) {
                auto pattern = GradientPattern::create(node);
                if (!pattern) {
                    return ll::forwardError(pattern.error());
                }
                return std::static_pointer_cast<Pattern>(*pattern);
            }
            if constexpr (std::is_same_v<T, BlockListPatternAst>) {
                auto pattern = BlockListPattern::create(node);
                if (!pattern) {
                    return ll::forwardError(pattern.error());
                }
                return std::static_pointer_cast<Pattern>(*pattern);
            }
        },
        ast
    );
}
} // namespace we
