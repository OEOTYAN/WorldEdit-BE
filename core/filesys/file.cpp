//
// Created by OEOTYAN on 2022/06/20.
//
#include "file.h"
#include <io.h>
#include <stdio.h>

namespace worldedit {
    void getFiles(std::string path, std::vector<std::string>& files) {
        intptr_t hFile = 0;
        struct _finddata_t fileinfo;
        std::string p;

        if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1) {
            do {
                if ((fileinfo.attrib & _A_SUBDIR)) {
                    if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                        getFiles(p.assign(path).append("/").append(fileinfo.name), files);
                } else {
                    files.push_back(p.assign(path).append("/").append(fileinfo.name));
                }
            } while (_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
        }
    }

    void getImageFiles(std::string path, std::vector<std::string>& files){
        getFiles(path, files);
        for (auto& file : files) {
            if (file.substr(file.length() - 4) != ".png") {
                files.erase(std::remove(files.begin(), files.end(), file), files.end());
            } else {
                file = file.substr(path.length()+1);
                file = file.substr(0, file.length()-4);
            }
        }
    }

}  // namespace worldedit