//
// Created by OEOTYAN on 2022/06/20.
//
#pragma once

#include "Globals.h"
#include <functional>
namespace worldedit {
    std::vector<std::string> getFiles(std::string path);
    std::vector<std::string> getWEFiles(std::string path);
    std::vector<std::string> getFiles(std::string path, std::function<bool(std::string const&)> filter);
    std::vector<std::string> getWEFiles(std::string path, std::function<bool(std::string const&)> filter);
}  // namespace worldedit
