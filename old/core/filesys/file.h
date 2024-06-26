#pragma once

#include "FileWatch.hpp"
#include "Globals.h"
#include <functional>
namespace we {
std::vector<std::string> getFiles(std::string path);
std::vector<std::string> getWEFiles(std::string path);
std::vector<std::string>
getFiles(std::string path, std::function<bool(std::string_view)> filter);
std::vector<std::string>
getWEFiles(std::string path, std::function<bool(std::string_view)> filter);
phmap::flat_hash_set<std::unique_ptr<class filewatch::FileWatch<std::wstring>>>&
getFileWatchSet();
void addFileWatch(std::string const& path, std::function<void(const std::filesystem::path&, const filewatch::Event)>);
} // namespace we
