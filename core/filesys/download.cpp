//
// Created by OEOTYAN on 2022/06/20.
//
#include "download.h"
#include <windows.h>
#include <wininet.h>
#include <urlmon.h>
#include <tchar.h>
#include <string>
#include <iostream>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4267)

namespace worldedit {
    int download(std::string const& url, std::string const& outfilename) {}

    bool downloadImage(std::string const& url) {
        size_t len = url.length();
        int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, (size_t)0);

        wchar_t* buffer = new wchar_t[nmlen];
        MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, (size_t)nmlen);

        DeleteUrlCacheEntryW(buffer);

        HRESULT hr = URLDownloadToFileW(NULL, buffer, _T("plugins/WorldEdit/imgtemp/0image"), 0, NULL);
        if (hr == S_OK) {
            return true;
        } else {
            return false;
        }
    }

}  // namespace worldedit