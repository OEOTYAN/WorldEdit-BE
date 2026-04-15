#include "pattern/PatternParser.h"

#include <cctype>
#include <charconv>

namespace we {
namespace {
std::string_view trimView(std::string_view input) {
    size_t begin = 0;
    size_t end   = input.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(input[begin]))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }
    return input.substr(begin, end - begin);
}

bool isBlockNameChar(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '.'
        || ch == '-';
}

bool isBlockNamespaceStartChar(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool isDigitString(std::string_view input) {
    if (input.empty()) {
        return false;
    }
    for (char ch : input) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

bool isTrivialNbtStringStartChar(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool isTrivialNbtStringChar(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '+'
        || ch == '_' || ch == '.';
}

std::optional<size_t>
findMatching(std::string_view input, size_t begin, char open, char close) {
    if (begin >= input.size() || input[begin] != open) {
        return std::nullopt;
    }

    int  depth          = 0;
    bool inSingleQuote  = false;
    bool inDoubleQuote  = false;
    bool escapeNextChar = false;
    for (size_t i = begin; i < input.size(); ++i) {
        char ch = input[i];
        if (escapeNextChar) {
            escapeNextChar = false;
            continue;
        }
        if (ch == '\\') {
            escapeNextChar = true;
            continue;
        }
        if (!inDoubleQuote && ch == '\'') {
            inSingleQuote = !inSingleQuote;
            continue;
        }
        if (!inSingleQuote && ch == '"') {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }
        if (inSingleQuote || inDoubleQuote) {
            continue;
        }
        if (ch == open) {
            ++depth;
        } else if (ch == close) {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    return std::nullopt;
}

std::vector<std::string_view> splitTopLevel(std::string_view input, char sep) {
    std::vector<std::string_view> result;
    size_t                        start          = 0;
    int                           parenDepth     = 0;
    int                           bracketDepth   = 0;
    int                           braceDepth     = 0;
    bool                          inSingleQuote  = false;
    bool                          inDoubleQuote  = false;
    bool                          escapeNextChar = false;
    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (escapeNextChar) {
            escapeNextChar = false;
            continue;
        }
        if (ch == '\\') {
            escapeNextChar = true;
            continue;
        }
        if (!inDoubleQuote && ch == '\'') {
            inSingleQuote = !inSingleQuote;
            continue;
        }
        if (!inSingleQuote && ch == '"') {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }
        if (inSingleQuote || inDoubleQuote) {
            continue;
        }
        if (ch == '(') ++parenDepth;
        else if (ch == ')') --parenDepth;
        else if (ch == '[') ++bracketDepth;
        else if (ch == ']') --bracketDepth;
        else if (ch == '{') ++braceDepth;
        else if (ch == '}') --braceDepth;
        else if (ch == sep && parenDepth == 0 && bracketDepth == 0 && braceDepth == 0) {
            auto token = trimView(input.substr(start, i - start));
            if (!token.empty()) {
                result.push_back(token);
            }
            start = i + 1;
        }
    }
    auto tail = trimView(input.substr(start));
    if (!tail.empty()) {
        result.push_back(tail);
    }
    return result;
}

ll::Expected<PatternExpr> parseExpr(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("empty expression");
    }
    if (value.front() == '(') {
        auto end = findMatching(value, 0, '(', ')');
        if (!end) {
            return ll::makeStringError("unterminated expression");
        }
        if (!trimView(value.substr(*end + 1)).empty()) {
            return ll::makeStringError("unexpected trailing characters after expression");
        }
        return PatternRuntimeExpr{std::string(value.substr(1, *end - 1))};
    }

    double literal = 0.0;
    auto   begin   = value.data();
    auto   end     = value.data() + value.size();
    auto   result  = std::from_chars(begin, end, literal);
    if (result.ec == std::errc{} && result.ptr == end) {
        return PatternLiteralExpr{literal};
    }
    return PatternRuntimeExpr{std::string(value)};
}

ll::Expected<std::optional<PatternExpr>> parseOptionalData(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return std::nullopt;
    }
    return parseExpr(value);
}

ll::Expected<std::string> parseSimpleString(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("expected string");
    }
    if (value.front() == '"') {
        bool escapeNextChar = false;
        for (size_t i = 1; i < value.size(); ++i) {
            char ch = value[i];
            if (escapeNextChar) {
                escapeNextChar = false;
                continue;
            }
            if (ch == '\\') {
                escapeNextChar = true;
                continue;
            }
            if (ch == '"') {
                if (!trimView(value.substr(i + 1)).empty()) {
                    return ll::makeStringError(
                        "unexpected trailing characters after string"
                    );
                }
                return std::string(value.substr(1, i - 1));
            }
        }
        return ll::makeStringError("unterminated string");
    }

    if (!isTrivialNbtStringStartChar(value.front())) {
        return ll::makeStringError("invalid unquoted string");
    }
    for (char ch : value) {
        if (!isTrivialNbtStringChar(ch)) {
            return ll::makeStringError("invalid unquoted string");
        }
    }
    return std::string(value);
}

ll::Expected<PatternStateValue> parseStateValue(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("state value is empty");
    }
    if (value == "true") {
        return PatternStateValue{true};
    }
    if (value == "false") {
        return PatternStateValue{false};
    }

    int  intValue = 0;
    auto intRes   = std::from_chars(value.data(), value.data() + value.size(), intValue);
    if (intRes.ec == std::errc{} && intRes.ptr == value.data() + value.size()) {
        return PatternStateValue{intValue};
    }

    bool maybeFloat = false;
    for (char ch : value) {
        if (ch == '.' || ch == 'e' || ch == 'E') {
            maybeFloat = true;
            break;
        }
    }
    if (maybeFloat) {
        try {
            size_t parsed     = 0;
            float  floatValue = std::stof(std::string(value), &parsed);
            if (parsed == value.size()) {
                return PatternStateValue{floatValue};
            }
        } catch (...) {}
    }

    auto stringValue = parseSimpleString(value);
    if (!stringValue) {
        return ll::forwardError(stringValue.error());
    }
    return PatternStateValue{stringValue.value()};
}

ll::Expected<PatternBlockStateAst> parseStateItem(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("empty block state item");
    }

    size_t equalsPos      = std::string::npos;
    bool   inSingleQuote  = false;
    bool   inDoubleQuote  = false;
    bool   escapeNextChar = false;
    for (size_t i = 0; i < value.size(); ++i) {
        char ch = value[i];
        if (escapeNextChar) {
            escapeNextChar = false;
            continue;
        }
        if (ch == '\\') {
            escapeNextChar = true;
            continue;
        }
        if (!inDoubleQuote && ch == '\'') {
            inSingleQuote = !inSingleQuote;
            continue;
        }
        if (!inSingleQuote && ch == '"') {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }
        if (!inSingleQuote && !inDoubleQuote && ch == '=') {
            equalsPos = i;
            break;
        }
    }
    if (equalsPos == std::string::npos) {
        return ll::makeStringError("block state item must contain '='");
    }

    auto key = parseSimpleString(value.substr(0, equalsPos));
    if (!key) {
        return ll::forwardError(key.error());
    }
    auto parsedValue = parseStateValue(value.substr(equalsPos + 1));
    if (!parsedValue) {
        return ll::forwardError(parsedValue.error());
    }
    return PatternBlockStateAst{key.value(), parsedValue.value()};
}

ll::Expected<std::vector<PatternBlockStateAst>> parseStateList(std::string_view input) {
    std::vector<PatternBlockStateAst> result;
    for (auto token : splitTopLevel(trimView(input), ',')) {
        auto state = parseStateItem(token);
        if (!state) {
            return ll::forwardError(state.error());
        }
        result.push_back(std::move(state.value()));
    }
    return result;
}

ll::Expected<PatternSimpleBlockSpec> parseSimpleBlock(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("empty block spec");
    }
    if (value.front() == '(') {
        auto end = findMatching(value, 0, '(', ')');
        if (!end) {
            return ll::makeStringError("unterminated block expression");
        }
        if (!trimView(value.substr(*end + 1)).empty()) {
            return ll::makeStringError(
                "unexpected trailing characters after block expression"
            );
        }
        PatternDynamicBlockAst block;
        block.source = PatternRuntimeExpr{std::string(value.substr(1, *end - 1))};
        return block;
    }
    if (isDigitString(value)) {
        double literal = 0.0;
        auto   begin   = value.data();
        auto   end     = value.data() + value.size();
        auto   result  = std::from_chars(begin, end, literal);
        if (result.ec == std::errc{} && result.ptr == end) {
            PatternDynamicBlockAst block;
            block.source = PatternLiteralExpr{literal};
            return block;
        }
    }
    if (value.front() == '{') {
        auto end = findMatching(value, 0, '{', '}');
        if (!end) {
            return ll::makeStringError("unterminated SNBT block");
        }
        if (!trimView(value.substr(*end + 1)).empty()) {
            return ll::makeStringError("unexpected trailing characters after SNBT block");
        }
        return PatternSnbtBlockAst{std::string(value.substr(0, *end + 1))};
    }

    size_t i = 0;
    while (i < value.size() && isBlockNameChar(value[i])) {
        ++i;
    }
    if (i < value.size() && value[i] == ':' && i + 1 < value.size()
        && isBlockNamespaceStartChar(value[i + 1])) {
        ++i;
        while (i < value.size() && isBlockNameChar(value[i])) {
            ++i;
        }
    }
    if (i == 0) {
        return ll::makeStringError("invalid block spec");
    }

    PatternNamedBlockAst block;
    block.name = std::string(value.substr(0, i));

    auto tail = trimView(value.substr(i));
    if (!tail.empty()) {
        if (tail.front() != '[') {
            return ll::makeStringError("unexpected trailing characters after block spec");
        }
        auto end = findMatching(tail, 0, '[', ']');
        if (!end) {
            return ll::makeStringError("unterminated block state list");
        }
        if (!trimView(tail.substr(*end + 1)).empty()) {
            return ll::makeStringError(
                "unexpected trailing characters after block state list"
            );
        }
        auto states = parseStateList(tail.substr(1, *end - 1));
        if (!states) {
            return ll::forwardError(states.error());
        }
        block.states = std::move(states.value());
    }
    return block;
}

ll::Expected<std::pair<PatternSimpleBlockSpec, std::string_view>>
parseSimpleBlockWithTail(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("empty block spec");
    }
    if (value.front() == '(') {
        auto end = findMatching(value, 0, '(', ')');
        if (!end) {
            return ll::makeStringError("unterminated block expression");
        }
        PatternDynamicBlockAst block;
        block.source = PatternRuntimeExpr{std::string(value.substr(1, *end - 1))};
        return std::pair{
            PatternSimpleBlockSpec{std::move(block)},
            trimView(value.substr(*end + 1))
        };
    }
    if (!value.empty() && std::isdigit(static_cast<unsigned char>(value.front()))) {
        size_t i = 0;
        while (i < value.size() && std::isdigit(static_cast<unsigned char>(value[i]))) {
            ++i;
        }

        PatternDynamicBlockAst block;
        double                 literal = 0.0;
        auto                   begin   = value.data();
        auto                   end     = value.data() + i;
        auto                   result  = std::from_chars(begin, end, literal);
        if (result.ec != std::errc{} || result.ptr != end) {
            return ll::makeStringError("invalid numeric block id");
        }
        block.source = PatternLiteralExpr{literal};

        auto tail = trimView(value.substr(i));
        if (!tail.empty() && tail.front() == ':') {
            auto data = parseOptionalData(tail.substr(1));
            if (!data) {
                return ll::forwardError(data.error());
            }
            block.data = std::move(data.value());
            tail       = std::string_view{};
        }

        return std::pair{PatternSimpleBlockSpec{std::move(block)}, trimView(tail)};
    }
    if (value.front() == '{') {
        auto end = findMatching(value, 0, '{', '}');
        if (!end) {
            return ll::makeStringError("unterminated SNBT block");
        }
        return std::pair{
            PatternSimpleBlockSpec{
                PatternSnbtBlockAst{std::string(value.substr(0, *end + 1))}
            },
            trimView(value.substr(*end + 1))
        };
    }

    size_t i = 0;
    while (i < value.size() && isBlockNameChar(value[i])) {
        ++i;
    }
    if (i < value.size() && value[i] == ':' && i + 1 < value.size()
        && isBlockNamespaceStartChar(value[i + 1])) {
        ++i;
        while (i < value.size() && isBlockNameChar(value[i])) {
            ++i;
        }
    }
    if (i == 0) {
        return ll::makeStringError("invalid block spec");
    }

    PatternNamedBlockAst block;
    block.name = std::string(value.substr(0, i));

    auto tail = trimView(value.substr(i));
    if (!tail.empty() && tail.front() == '[') {
        auto end = findMatching(tail, 0, '[', ']');
        if (!end) {
            return ll::makeStringError("unterminated block state list");
        }
        auto states = parseStateList(tail.substr(1, *end - 1));
        if (!states) {
            return ll::forwardError(states.error());
        }
        block.states = std::move(states.value());
        tail         = trimView(tail.substr(*end + 1));
    }

    if (!tail.empty() && tail.front() == ':') {
        auto data = parseOptionalData(tail.substr(1));
        if (!data) {
            return ll::forwardError(data.error());
        }
        block.data = std::move(data.value());
        tail       = std::string_view{};
    }

    return std::pair{PatternSimpleBlockSpec{std::move(block)}, trimView(tail)};
}

ll::Expected<PatternBlockSpec> parseBlockSpec(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("empty block spec");
    }
    if (value.front() == '[') {
        auto end = findMatching(value, 0, '[', ']');
        if (!end) {
            return ll::makeStringError("unterminated packed block spec");
        }
        if (!trimView(value.substr(*end + 1)).empty()) {
            return ll::makeStringError(
                "unexpected trailing characters after packed block spec"
            );
        }

        auto items = splitTopLevel(value.substr(1, *end - 1), ',');
        if (items.empty() || items.size() > 3) {
            return ll::makeStringError("packed block spec supports 1 to 3 items");
        }

        PatternPackedBlockSpec packed;
        auto                   block = parseSimpleBlockWithTail(items[0]);
        if (!block) {
            return ll::forwardError(block.error());
        }
        if (!trimView(block.value().second).empty()) {
            return ll::makeStringError(
                "unexpected trailing characters after packed block item"
            );
        }
        packed.block = std::move(block.value().first);

        if (items.size() >= 2) {
            auto extra = parseSimpleBlockWithTail(items[1]);
            if (!extra) {
                return ll::forwardError(extra.error());
            }
            if (!trimView(extra.value().second).empty()) {
                return ll::makeStringError(
                    "unexpected trailing characters after packed extra block item"
                );
            }
            packed.extraBlock = std::move(extra.value().first);
        }

        if (items.size() == 3) {
            auto entity = trimView(items[2]);
            if (entity.empty() || entity.front() != '{') {
                return ll::makeStringError("packed block entity must be SNBT");
            }
            auto entityEnd = findMatching(entity, 0, '{', '}');
            if (!entityEnd || !trimView(entity.substr(*entityEnd + 1)).empty()) {
                return ll::makeStringError("invalid packed block entity SNBT");
            }
            packed.blockEntity = std::string(entity.substr(0, *entityEnd + 1));
        }
        return PatternBlockSpec{std::move(packed)};
    }

    auto simple = parseSimpleBlockWithTail(value);
    if (!simple) {
        return ll::forwardError(simple.error());
    }
    if (!trimView(simple.value().second).empty()) {
        return ll::makeStringError("unexpected trailing characters after block spec");
    }
    return PatternBlockSpec{std::move(simple.value().first)};
}

ll::Expected<std::pair<PatternBlockSpec, std::string_view>>
parseBlockSpecWithTail(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("empty block spec");
    }
    if (value.front() == '[') {
        auto end = findMatching(value, 0, '[', ']');
        if (!end) {
            return ll::makeStringError("unterminated packed block spec");
        }
        auto block = parseBlockSpec(value.substr(0, *end + 1));
        if (!block) {
            return ll::forwardError(block.error());
        }
        return std::pair{std::move(block.value()), trimView(value.substr(*end + 1))};
    }

    auto simple = parseSimpleBlockWithTail(value);
    if (!simple) {
        return ll::forwardError(simple.error());
    }
    return std::pair{
        PatternBlockSpec{std::move(simple.value().first)},
        simple.value().second
    };
}

ll::Expected<ClipboardPatternAst> parseClipboardPattern(std::string_view input) {
    ClipboardPatternAst ast;
    auto                value = trimView(input);
    if (!value.starts_with("#clipboard")) {
        return ll::makeStringError("invalid clipboard pattern");
    }

    auto suffix = trimView(value.substr(10));
    if (suffix.empty()) {
        return ast;
    }
    if (!suffix.starts_with("[") || !suffix.ends_with("]")) {
        return ll::makeStringError("clipboard pattern arguments must be wrapped by []");
    }

    auto inner = trimView(suffix.substr(1, suffix.size() - 2));
    if (inner.empty()) {
        return ast;
    }
    if (inner == "@c") {
        ast.useCenter = true;
        return ast;
    }

    auto tokens = splitTopLevel(inner, ',');
    if (tokens.size() > 3) {
        return ll::makeStringError("clipboard offset supports at most 3 integers");
    }
    for (size_t i = 0; i < tokens.size(); ++i) {
        auto token = trimView(tokens[i]);
        auto result =
            std::from_chars(token.data(), token.data() + token.size(), ast.offset[i]);
        if (result.ec != std::errc{} || result.ptr != token.data() + token.size()) {
            return ll::makeStringError("clipboard offset must be integer");
        }
    }
    return ast;
}

ll::Expected<GradientPatternAst>
parseGradientPattern(std::string_view input, bool lighten) {
    GradientPatternAst ast;
    ast.lighten = lighten;

    auto prefix = lighten ? std::string_view{"#lighten"} : std::string_view{"#darken"};
    auto value  = trimView(input);
    if (!value.starts_with(prefix)) {
        return ll::makeStringError("invalid gradient pattern");
    }

    auto suffix = trimView(value.substr(prefix.size()));
    if (suffix.empty()) {
        return ll::makeStringError("gradient pattern requires selectors");
    }
    if (!suffix.starts_with("[") || !suffix.ends_with("]")) {
        return ll::makeStringError("gradient pattern arguments must be wrapped by []");
    }

    auto inner = trimView(suffix.substr(1, suffix.size() - 2));
    if (inner.empty()) {
        return ll::makeStringError("gradient pattern requires at least one selector");
    }

    for (auto token : splitTopLevel(inner, ',')) {
        auto selector = trimView(token);
        if (!selector.empty()) {
            ast.selectors.emplace_back(selector);
        }
    }
    if (ast.selectors.empty()) {
        return ll::makeStringError("gradient pattern requires at least one selector");
    }
    return ast;
}

ll::Expected<PatternWeightedEntry> parseEntry(std::string_view input) {
    auto text = trimView(input);
    if (text.empty()) {
        return ll::makeStringError("empty pattern entry");
    }

    size_t percentPos     = std::string::npos;
    bool   inSingleQuote  = false;
    bool   inDoubleQuote  = false;
    bool   escapeNextChar = false;
    int    parenDepth     = 0;
    int    bracketDepth   = 0;
    int    braceDepth     = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        char ch = text[i];
        if (escapeNextChar) {
            escapeNextChar = false;
            continue;
        }
        if (ch == '\\') {
            escapeNextChar = true;
            continue;
        }
        if (!inDoubleQuote && ch == '\'') {
            inSingleQuote = !inSingleQuote;
            continue;
        }
        if (!inSingleQuote && ch == '"') {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }
        if (inSingleQuote || inDoubleQuote) {
            continue;
        }
        if (ch == '(') ++parenDepth;
        else if (ch == ')') --parenDepth;
        else if (ch == '[') ++bracketDepth;
        else if (ch == ']') --bracketDepth;
        else if (ch == '{') ++braceDepth;
        else if (ch == '}') --braceDepth;
        else if (ch == '%' && parenDepth == 0 && bracketDepth == 0 && braceDepth == 0) {
            percentPos = i;
            break;
        }
    }

    PatternExpr      weight = PatternLiteralExpr{1.0};
    std::string_view remain = text;
    if (percentPos != std::string::npos) {
        auto expr = parseExpr(text.substr(0, percentPos));
        if (!expr) {
            return ll::forwardError(expr.error());
        }
        weight = expr.value();
        remain = trimView(text.substr(percentPos + 1));
    }

    auto block = parseBlockSpec(remain);
    if (!block) {
        return ll::forwardError(block.error());
    }

    return PatternWeightedEntry{weight, std::move(block.value())};
}
} // namespace

ll::Expected<PatternAst> PatternParser::parse(std::string_view input) {
    input = trimView(input);
    if (input.size() >= 2 && input.front() == '"' && input.back() == '"') {
        input = trimView(input.substr(1, input.size() - 2));
    }

    if (input == "#hand") {
        return PatternAst{HandPatternAst{}};
    }
    if (input.starts_with("#clipboard")) {
        auto clipboard = parseClipboardPattern(input);
        if (!clipboard) {
            return ll::forwardError(clipboard.error());
        }
        return PatternAst{std::move(clipboard.value())};
    }
    if (input.starts_with("#lighten")) {
        auto gradient = parseGradientPattern(input, true);
        if (!gradient) {
            return ll::forwardError(gradient.error());
        }
        return PatternAst{std::move(gradient.value())};
    }
    if (input.starts_with("#darken")) {
        auto gradient = parseGradientPattern(input, false);
        if (!gradient) {
            return ll::forwardError(gradient.error());
        }
        return PatternAst{std::move(gradient.value())};
    }

    BlockListPatternAst ast;
    for (auto token : splitTopLevel(input, ',')) {
        auto entry = parseEntry(token);
        if (!entry) {
            return ll::forwardError(entry.error());
        }
        ast.entries.push_back(std::move(entry.value()));
    }
    if (ast.entries.empty()) {
        return ll::makeStringError("pattern is empty");
    }
    return PatternAst{std::move(ast)};
}
} // namespace we
