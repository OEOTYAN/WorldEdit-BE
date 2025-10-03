add_rules("mode.release")

add_rules("plugin.compile_commands.autoupdate", {outputdir=".vscode"})

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("oeo-repo https://github.com/OEOTYAN/xmake-repo.git")


add_requires("levilamina 435ae9836047c827003bad585f1f452c24bda779")
add_requires("levibuildscript")

add_requires("bsci")
add_requires("tinyobjloader")
add_requires("exprtk")
-- add_requires("openimageio")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("WorldEdit") -- Change this to your mod name.
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("_HAS_CXX23=1", "NOMINMAX", "UNICODE")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_files("thirdparty/**.cpp")
    add_includedirs("thirdparty")
    set_pcxxheader("src/worldedit/Global.h")
    add_packages("levilamina", "bsci", "tinyobjloader", "exprtk") --, "openimageio")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
