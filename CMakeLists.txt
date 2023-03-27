cmake_minimum_required(VERSION 3.21)
project(WorldEdit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

file(GLOB_RECURSE SRC_FILES_DIR
    ${PROJECT_SOURCE_DIR}/core/*/*.cpp
    ${PROJECT_SOURCE_DIR}/WorldEdit/*.cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/*/*.cpp
)
include_directories(core)
include_directories(thirdparty)
include_directories(thirdparty/lodepng)
include_directories(thirdparty/mixbox)
include_directories(SDK/include/llapi)
include_directories(SDK/include)
include_directories(WorldEdit)

add_definitions(
    -D"NDEBUG"
    -D"NOMINMAX"
    -D"TEMPLATE_EXPORTS"
    -D"UNICODE"
    -D"WIN32_LEAN_AND_MEAN"
    -D"_AMD64_"
    -D"_CRT_SECURE_NO_WARNINGS"
    -D"_UNICODE"
    -D"_USRDLL"
    -D"_WINDLL"
    -D"_WINDOWS")

add_compile_options(
    /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Zf /Gm- /sdl /Zc:inline /fp:precise /errorReport:prompt /WX- /utf-8
    /Zc:forScope /Gd /Oi /Ot /MD /std:c++20 /FC /EHsc /nologo /diagnostics:column
)

add_link_options(
    /MANIFEST /LTCG:incremental /NXCOMPAT /DEBUG /DLL /MACHINE:X64 /OPT:REF /SUBSYSTEM:CONSOLE
    /MANIFESTUAC:NO /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /DELAYLOAD:"bedrock_server.dll" /TLBID:1
)

add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC_FILES_DIR})

target_link_libraries(${CMAKE_PROJECT_NAME}
    "${CMAKE_SOURCE_DIR}/SDK/lib/LiteLoader.lib"
    "${CMAKE_SOURCE_DIR}/SDK/lib/bedrock_server_api.lib"
    "${CMAKE_SOURCE_DIR}/SDK/lib/bedrock_server_var.lib"
    "${CMAKE_SOURCE_DIR}/SDK/lib/SymDBHelper.lib"
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
    COMMAND cmd /c ${PROJECT_SOURCE_DIR}/prepareLib.cmd ${PROJECT_SOURCE_DIR}
    COMMENT "Preparing Library"
)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND cmd /c ${PROJECT_SOURCE_DIR}/runbds.bat ${PROJECT_SOURCE_DIR}
)
