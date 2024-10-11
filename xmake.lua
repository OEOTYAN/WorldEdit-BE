add_rules("mode.release")

add_rules("plugin.compile_commands.autoupdate", {outputdir=".vscode"})

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("oeo-repo https://github.com/OEOTYAN/xmake-repo.git")

add_requires("levilamina develop")
add_requires("bsci main")
add_requires("parallel-hashmap")
-- add_requires("openimageio 2.5.10+1", {verify = false})
add_requires("tinyobjloader")
add_requires("levibuildscript 0.2.0")

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
    add_packages("levilamina", "bsci", "parallel-hashmap", "tinyobjloader")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
