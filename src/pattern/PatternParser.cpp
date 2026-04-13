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

std::optional<size_t> findParenthesizedEnd(std::string_view input, size_t begin) {
    if (begin >= input.size() || input[begin] != '(') {
        return std::nullopt;
    }
    int  parenDepth = 0;
    bool inQuote    = false;
    for (size_t i = begin; i < input.size(); ++i) {
        char ch = input[i];
        if (ch == '\'') {
            inQuote = !inQuote;
            continue;
        }
        if (inQuote) {
            continue;
        }
        if (ch == '(') {
            ++parenDepth;
        } else if (ch == ')') {
            --parenDepth;
            if (parenDepth == 0) {
                return i;
            }
        }
    }
    return std::nullopt;
}

std::vector<std::string_view> splitTopLevel(std::string_view input, char sep) {
    std::vector<std::string_view> result;
    auto                          start      = size_t{0};
    int                           parenDepth = 0;
    bool                          inString   = false;
    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (ch == '\'') {
            inString = !inString;
            continue;
        }
        if (inString) {
            continue;
        }
        if (ch == '(') {
            ++parenDepth;
        } else if (ch == ')') {
            --parenDepth;
        } else if (ch == sep && parenDepth == 0) {
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
        auto end = findParenthesizedEnd(value, 0);
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

ll::Expected<std::pair<PatternBlockSpec, std::string_view>>
parseBlockSpecWithTail(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return ll::makeStringError("empty block spec");
    }
    if (value.front() == '(') {
        auto end = findParenthesizedEnd(value, 0);
        if (!end) {
            return ll::makeStringError("unterminated block expression");
        }
        return std::pair{
            PatternBlockSpec{PatternRuntimeExpr{std::string(value.substr(1, *end - 1))}},
            trimView(value.substr(*end + 1))
        };
    }

    size_t i = 0;
    while (i < value.size() && isBlockNameChar(value[i])) {
        ++i;
    }
    if (i < value.size() && value[i] == ':' && i + 1 < value.size()
        && isBlockNameChar(value[i + 1])) {
        ++i;
        while (i < value.size() && isBlockNameChar(value[i])) {
            ++i;
        }
    }
    if (i == 0) {
        return ll::makeStringError("invalid block spec");
    }

    std::string blockName{value.substr(0, i)};
    return std::pair{PatternBlockSpec{std::move(blockName)}, trimView(value.substr(i))};
}

ll::Expected<std::optional<PatternExpr>> parseDataSpec(std::string_view input) {
    auto value = trimView(input);
    if (value.empty()) {
        return std::nullopt;
    }
    return parseExpr(value);
}

ll::Expected<PatternWeightedEntry> parseEntry(std::string_view input) {
    auto text = trimView(input);
    if (text.empty()) {
        return ll::makeStringError("empty pattern entry");
    }

    size_t percentPos = std::string::npos;
    bool   inString   = false;
    int    parenDepth = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        char ch = text[i];
        if (ch == '\'') {
            inString = !inString;
            continue;
        }
        if (inString) {
            continue;
        }
        if (ch == '(') ++parenDepth;
        else if (ch == ')') --parenDepth;
        else if (ch == '%' && parenDepth == 0) {
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
        weight = *expr;
        remain = trimView(text.substr(percentPos + 1));
    }

    PatternBlockSpec           block;
    std::optional<PatternExpr> data;
    auto                       parsedBlock = parseBlockSpecWithTail(remain);
    if (!parsedBlock) {
        return ll::forwardError(parsedBlock.error());
    }
    block = std::move(parsedBlock->first);

    auto tail = trimView(parsedBlock->second);
    if (!tail.empty()) {
        if (tail.front() != ':') {
            return ll::makeStringError("unexpected trailing characters after block spec");
        }
        auto parsedData = parseDataSpec(tail.substr(1));
        if (!parsedData) {
            return ll::forwardError(parsedData.error());
        }
        data = std::move(*parsedData);
    }

    return PatternWeightedEntry{weight, block, data};
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

    BlockListPatternAst ast;
    for (auto& token : splitTopLevel(input, ',')) {
        if (token.empty()) {
            continue;
        }
        auto entry = parseEntry(token);
        if (!entry) {
            return ll::forwardError(entry.error());
        }
        ast.entries.push_back(std::move(*entry));
    }
    if (ast.entries.empty()) {
        return ll::makeStringError("pattern is empty");
    }
    return PatternAst{std::move(ast)};
}
} // namespace we
