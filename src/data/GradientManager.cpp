#include "data/GradientManager.h"

#include "nlohmann/json.hpp"

#include <fstream>

namespace we {
static std::string pathToString(std::filesystem::path const& path) {
    return ll::string_utils::u8str2str(path.u8string());
}

static ll::Expected<std::string> serializeGradient(GradientData const& gradient) {
    if (gradient.blocks.empty()) {
        return ll::makeStringError("gradient json must not be empty");
    }

    nlohmann::json json = gradient.blocks;
    return json.dump(2);
}

static ll::Expected<std::vector<std::string>>
deserializeGradientBlocks(std::string_view text) {
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(text.begin(), text.end(), nullptr, true, true);
    } catch (...) {
        return ll::makeExceptionError();
    }

    if (!json.is_array()) {
        return ll::makeStringError("gradient json root must be array");
    }

    std::vector<std::string> blocks;
    for (auto const& node : json) {
        if (!node.is_string()) {
            return ll::makeStringError("gradient block entry must be string");
        }
        blocks.push_back(node.get<std::string>());
    }

    if (blocks.empty()) {
        return ll::makeStringError("gradient json must not be empty");
    }
    return blocks;
}

std::string
GradientManager::makeKey(std::string_view namespaceName, std::string_view gradientName) {
    return std::string(namespaceName) + ":" + std::string(gradientName);
}

ll::Expected<GradientData> GradientManager::loadFile(
    std::filesystem::path const& rootDir,
    std::filesystem::path const& filePath
) {
    GradientData gradient;
    auto         relative = std::filesystem::relative(filePath, rootDir);
    if (relative.empty() || relative.extension() != ".json"
        || relative.begin() == relative.end()) {
        return ll::makeStringError(
            "invalid gradient file path: " + pathToString(filePath)
        );
    }

    auto iter              = relative.begin();
    gradient.namespaceName = ll::string_utils::u8str2str(iter->u8string());
    ++iter;
    if (iter == relative.end()) {
        return ll::makeStringError(
            "gradient file must be under namespace directory: " + pathToString(filePath)
        );
    }

    auto leaf             = relative.filename();
    gradient.gradientName = pathToString(leaf.stem());

    std::ifstream input(filePath);
    if (!input.is_open()) {
        return ll::makeStringError(
            "failed to open gradient file: " + pathToString(filePath)
        );
    }
    std::string text{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    auto blocks = deserializeGradientBlocks(text);
    if (!blocks) {
        return ll::forwardError(blocks.error());
    }
    gradient.blocks = std::move(*blocks);
    return gradient;
}

ll::Expected<> GradientManager::writeFile(
    std::filesystem::path const& rootDir,
    GradientData const&          gradient
) {
    auto json = serializeGradient(gradient);
    if (!json) {
        return ll::forwardError(json.error());
    }

    auto            namespaceDir = rootDir / gradient.namespaceName;
    std::error_code ec;
    std::filesystem::create_directories(namespaceDir, ec);
    if (ec) {
        return ll::makeStringError(
            "failed to create gradient namespace directory: " + pathToString(namespaceDir)
        );
    }

    auto          filePath = namespaceDir / (gradient.gradientName + ".json");
    std::ofstream output(filePath, std::ios::trunc);
    if (!output.is_open()) {
        return ll::makeStringError(
            "failed to open gradient file for write: " + pathToString(filePath)
        );
    }
    output << *json;
    return {};
}

ll::Expected<> GradientManager::loadAll() {
    gradients.clear();
    auto rootDir = WorldEdit::getInstance().getGradientDataPath();
    if (!std::filesystem::exists(rootDir)) {
        return {};
    }

    for (auto const& entry : std::filesystem::recursive_directory_iterator(rootDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        auto gradient = loadFile(rootDir, entry.path());
        if (!gradient) {
            return ll::forwardError(gradient.error());
        }
        registerGradient(std::move(*gradient));
    }
    return {};
}

ll::Expected<> GradientManager::save(GradientData const& gradient) const {
    return writeFile(WorldEdit::getInstance().getGradientDataPath(), gradient);
}

ll::Expected<> GradientManager::saveAll() const {
    auto rootDir = WorldEdit::getInstance().getGradientDataPath();
    for (auto const& [_, gradient] : gradients) {
        auto result = writeFile(rootDir, gradient);
        if (!result) {
            return ll::forwardError(result.error());
        }
    }
    return {};
}

bool GradientManager::contains(
    std::string_view namespaceName,
    std::string_view gradientName
) const {
    return gradients.contains(makeKey(namespaceName, gradientName));
}

optional_ref<GradientData const> GradientManager::tryGet(
    std::string_view namespaceName,
    std::string_view gradientName
) const {
    auto iter = gradients.find(makeKey(namespaceName, gradientName));
    return iter == gradients.end() ? nullptr
                                   : optional_ref<GradientData const>(iter->second);
}

void GradientManager::registerGradient(GradientData gradient) {
    gradients.insert_or_assign(
        makeKey(gradient.namespaceName, gradient.gradientName),
        std::move(gradient)
    );
}

ll::coro::Generator<GradientData const&> GradientManager::values() const {
    for (auto const& [_, gradient] : gradients) {
        co_yield gradient;
    }
}
} // namespace we