add_rules("mode.release")

add_rules("plugin.compile_commands.autoupdate", {outputdir=".vscode"})

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

package("bscidev")
    add_urls("https://github.com/OEOTYAN/BedrockServerClientInterface.git")
    add_versions("0.1.5", "50503ed2f734a9d13145b38d9a18961d7e8ac8d3")
    on_install(function (package)
        import("package.tools.xmake").install(package)
    end)
package_end()

add_requires("levilamina develop")
add_requires("bscidev")
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
    add_packages("levilamina", "bscidev", "parallel-hashmap", "tinyobjloader")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
