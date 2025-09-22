#include "HistoryRecordFactory.h"

namespace we {

std::unordered_map<std::string, HistoryRecordFactory::CreatorFunction>
    HistoryRecordFactory::sCreators;

void HistoryRecordFactory::registerCreator(
    const std::string& type,
    CreatorFunction    creator
) {
    sCreators[type] = std::move(creator);
}

std::unique_ptr<HistoryRecord> HistoryRecordFactory::create(const std::string& type) {
    auto it = sCreators.find(type);
    if (it != sCreators.end()) {
        return it->second();
    }
    return nullptr;
}

bool HistoryRecordFactory::isSupported(const std::string& type) {
    return sCreators.find(type) != sCreators.end();
}

std::vector<std::string> HistoryRecordFactory::getSupportedTypes() {
    std::vector<std::string> types;
    types.reserve(sCreators.size());

    for (const auto& pair : sCreators) {
        types.push_back(pair.first);
    }

    return types;
}

} // namespace we