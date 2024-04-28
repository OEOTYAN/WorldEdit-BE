add_rules("mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("levilamina")
add_requires("bsci")
add_requires("parallel-hashmap")
-- add_requires("openimageio 2.5.10+1", {verify = false})
add_requires("tinyobjloader")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("WorldEdit") -- Change this to your plugin name.
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204"
    ) 
    add_defines("_HAS_CXX23=1", "NOMINMAX", "UNICODE")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_files("thirdparty/**.cpp")
    add_includedirs("thirdparty")
    set_pcxxheader("src/worldedit/Global.h")
    add_packages("levilamina", "bsci", "parallel-hashmap", "boost", "tinyobjloader")
    add_shflags("/DELAYLOAD:bedrock_server.dll") -- To use symbols provided by SymbolProvider.
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local tag = os.iorun("git describe --tags --abbrev=0 --always")
        local major, minor, patch, suffix = tag:match(".*(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            print("Failed to parse version tag, using 0.0.0")
            major, minor, patch = 0, 0, 0
        end
        local plugin_define = {
            pluginName = target:name(),
            pluginFile = path.filename(target:targetfile()),
            pluginVersion = major .. "." .. minor .. "." .. patch,
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)
