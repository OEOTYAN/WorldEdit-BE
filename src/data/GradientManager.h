#pragma once

#include "data/GradientData.h"
#include "worldedit/WorldEdit.h"

namespace we {
class GradientManager {
    ll::DenseMap<std::string, GradientData> gradients;

    static std::string
    makeKey(std::string_view namespaceName, std::string_view gradientName);
    static ll::Expected<GradientData>
    loadFile(std::filesystem::path const& rootDir, std::filesystem::path const& filePath);
    static ll::Expected<>
    writeFile(std::filesystem::path const& rootDir, GradientData const& gradient);

public:
    ll::Expected<> loadAll();
    ll::Expected<> save(GradientData const& gradient) const;
    ll::Expected<> saveAll() const;

    bool contains(std::string_view namespaceName, std::string_view gradientName) const;
    optional_ref<GradientData const>
         tryGet(std::string_view namespaceName, std::string_view gradientName) const;
    void registerGradient(GradientData gradient);
    ll::coro::Generator<GradientData const&> values() const;
};
} // namespace we
