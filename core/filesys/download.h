//
// Created by OEOTYAN on 2022/06/20.
//
#pragma once
#ifndef WORLDEDIT_DOWNLOAD_H
#define WORLDEDIT_DOWNLOAD_H

#include "Global.h"
namespace worldedit {
    int download(std::string const& url, std::string const& outfilename);
    bool downloadImage(std::string const& url);
}  // namespace worldedit

#endif  // WORLDEDIT_DOWNLOAD_H