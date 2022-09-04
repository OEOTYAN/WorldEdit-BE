//
// Created by OEOTYAN on 2022/06/20.
//
#pragma once
#ifndef WORLDEDIT_FILE_H
#define WORLDEDIT_FILE_H

#include "Global.h"
namespace worldedit {
    void getFiles(std::string path, std::vector<std::string>& files);
    void getImageFiles(std::string path, std::vector<std::string>& files);
}  // namespace worldedit

#endif  // WORLDEDIT_FILE_H