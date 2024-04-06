#include "download.h"
#include <iostream>
#include <string>
#include <tchar.h>
#include <urlmon.h>
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4267)

namespace we {
int download(std::string_view url, std::string_view outfilename) {}

bool downloadImage(std::string const& url) {
    size_t len   = url.length();
    int    nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, (size_t)0);

    wchar_t* buffer = new wchar_t[nmlen];
    MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, (size_t)nmlen);

    DeleteUrlCacheEntryW(buffer);

    HRESULT hr =
        URLDownloadToFileW(NULL, buffer, _T("plugins/WorldEdit/imgtemp/0image"), 0, NULL);
    if (hr == S_OK) {
        return true;
    } else {
        return false;
    }
}

} // namespace we