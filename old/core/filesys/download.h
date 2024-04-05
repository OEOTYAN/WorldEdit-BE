//
// Created by OEOTYAN on 2022/06/20.
//
#pragma once

#include "Globals.h"
namespace worldedit {
    int download(std::string_view url, std::string_view outfilename);
    bool downloadImage(std::string const& url);
}  // namespace worldedit
