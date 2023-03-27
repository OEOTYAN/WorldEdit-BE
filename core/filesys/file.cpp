//
// Created by OEOTYAN on 2022/06/20.
//
#include "file.h"
#include <iostream>
#include <filesystem>
#include <utils/StringHelper.h>
#include <WorldEdit.h>

namespace worldedit {
    std::vector<std::string> getFiles(std::string path) {
        std::vector<std::string> res;
        res.clear();
        auto length = path.length();
        if (path.back() != '\\' && path.back() != '/') {
            length += 1;
        }
        for (auto& file : std::filesystem::recursive_directory_iterator(path)) {
            const auto& str = UTF82String(file.path().u8string()).substr(length);
            // std::cout << str << std::endl;
            res.push_back(str);
        }
        return res;
    }
    std::vector<std::string> getWEFiles(std::string path) {
        return getFiles(WE_DIR + path);
    }
    std::vector<std::string> getFiles(std::string path, std::function<bool(std::string_view)> filter) {
        std::vector<std::string> res;
        res.clear();
        auto length = path.length();
        if (path.back() != '\\' && path.back() != '/') {
            length += 1;
        }
        for (auto& file : std::filesystem::recursive_directory_iterator(path)) {
            const auto& str = UTF82String(file.path().u8string()).substr(length);
            if (filter(str)) {
                res.push_back(str);
            }
        }
        return res;
    }
    std::vector<std::string> getWEFiles(std::string path, std::function<bool(std::string_view)> filter) {
        return getFiles(WE_DIR + path, filter);
    }
    phmap::flat_hash_set<std::unique_ptr<class filewatch::FileWatch<std::wstring>>>& getFileWatchSet() {
        static phmap::flat_hash_set<std::unique_ptr<class filewatch::FileWatch<std::wstring>>> sets;
        return sets;
    }

    void
    addFileWatch(std::string const& path,
                 std::function<void(const std::filesystem::path&, const filewatch::Event)> func) {
        getFileWatchSet().insert(std::unique_ptr<filewatch::FileWatch<std::wstring>>(
            new filewatch::FileWatch<std::wstring>(str2wstr(path), func)));
    }
}  // namespace worldedit