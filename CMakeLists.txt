cmake_minimum_required(VERSION 3.21)
project(WorldEdit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

file(GLOB_RECURSE SRC_FILES_DIR
        ${PROJECT_SOURCE_DIR}/SDK/Header/*.cpp
        ${PROJECT_SOURCE_DIR}/SDK/Header/*.hpp
        ${PROJECT_SOURCE_DIR}/SDK/Header/*.h
        ${PROJECT_SOURCE_DIR}/core/*/*.cpp
        ${PROJECT_SOURCE_DIR}/core/*/*.hpp
        ${PROJECT_SOURCE_DIR}/core/*/*.h
        ${PROJECT_SOURCE_DIR}/Template/*.cpp
        ${PROJECT_SOURCE_DIR}/Template/*.hpp
        ${PROJECT_SOURCE_DIR}/Template/*.h
        ${PROJECT_SOURCE_DIR}/lodepng/*.cpp
        ${PROJECT_SOURCE_DIR}/lodepng/*.h
        ${PROJECT_SOURCE_DIR}/FastNoise/*.h
        )
include_directories(core)
include_directories(FastNoise)
include_directories(lodepng)
include_directories(SDK/Header)
include_directories(SDK/Header/third-party)
include_directories(Template)

link_directories(.)

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
    /diagnostics:column
    /EHsc
    /FC
    /GL
    /MD
    /nologo
    /permissive-
    /sdl
    /utf-8
    /Zc:inline
    /Zi
)

add_link_options(
    /DEBUG
    /DELAYLOAD:"bedrock_server.dll"
    /DLL
    /IGNORE:4199
    /INCREMENTAL:NO
    /LTCG:INCREMENTAL
    /MANIFESTUAC:NO
    /MACHINE:X64
    /NOLOGO
    /OPT:ICF
    /OPT:REF
    /SUBSYSTEM:CONSOLE
)

add_library(WorldEdit SHARED ${SRC_FILES_DIR})

add_custom_command(TARGET WorldEdit PRE_BUILD
        COMMAND cmd /c ${PROJECT_SOURCE_DIR}/prepareLib.cmd ${PROJECT_SOURCE_DIR}
        COMMENT "Preparing Library"
        )
