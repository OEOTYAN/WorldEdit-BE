// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "Globals.h"
#include <LLAPI.h>
#include "Version.h"
#pragma comment(lib, "../SDK/Lib/bedrock_server_api.lib")
#pragma comment(lib, "../SDK/Lib/bedrock_server_var.lib")
#pragma comment(lib, "../SDK/Lib/SymDBHelper.lib")
#pragma comment(lib, "../SDK/Lib/LiteLoader.lib")

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            ll::registerPlugin(PLUGIN_NAME, PLUGIN_INTRODUCTION,
                               ll::Version(PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_REVISION,
                                           static_cast<ll::Version::Status>(PLUGIN_LLVERSION_STATUS)),
                               std::map<std::string, std::string>{
#ifdef PLUGIN_AUTHOR
                                   {"Author", PLUGIN_AUTHOR},
#endif
                               });
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

void PluginInit();

extern "C" {
// Do something after all the plugins loaded
_declspec(dllexport) void onPostInit() {
    std::ios::sync_with_stdio(false);
    PluginInit();
}
}
