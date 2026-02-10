set_xmakever("2.9.5")

-- Globals
PROJECT_NAME = "SexLabUtil"
PROJECT_VERSION = "2.16.0"

-- Includes
includes("lib/CommonLibSSE-NG/xmake.lua")
includes("xmake/dotenv")
includes("xmake/papyrus")
includes("xmake/spriggit")
add_moduledirs("xmake/modules")

-- Project
set_project(PROJECT_NAME)
set_version(PROJECT_VERSION)
set_languages("cxx23")
set_license("apache-2.0")
set_warnings("allextra", "error")

-- Options
option("papyrus_include")
    set_category("papyrus")
    set_description("Path to papyrus include directories",
                    'Defaults to: "%XSE_TES5_MODS_PATH%"')
    on_check(function(option)
        import("dotenv.check")(option, {"$(env XSE_TES5_MODS_PATH)"})
    end)
option_end()

option("papyrus_gamesource")
    set_category("papyrus")
    set_description('Path to directory containing game\'s "Source\\Scripts"',
                    'Defaults to: "%XSE_TES5_GAME_PATH%\\Data"')
    on_check(function(option)
        import("dotenv.check")(option, {"$(env XSE_TES5_GAME_PATH)", "Data"})
    end)
option_end()

option("auto_install")
    set_category("install")
    set_description("Automatically trigger an install after successful build",
                    "Only if 'install_path' is set")
    set_default(false)
    after_check("dotenv.check")
option_end()

option("install_path")
    set_category("install")
    set_description("Set default installation path for commands and auto_install",
                    'Defaults to: "%XSE_TES5_MODS_PATH%\\SL-Dev"')
    on_check(function(option)
        import("dotenv.check")(option, {"$(env XSE_TES5_MODS_PATH)", "SL-Dev"})
    end)
option_end()

option("build_assets")
    set_category("build")
    set_description("Enable 'assets' target")
    set_default(true)
option("build_dll")
    set_category("build")
    set_description("Enable '" .. PROJECT_NAME .. "' target")
    set_default(true)
option("build_papyrus")
    set_category("build")
    set_description("Enable 'papyrus' target")
    set_default(true)
option("build_spriggit")
    set_category("build")
    set_description("Enable 'SexLab.esm' target")
    set_default(true)

-- ensure dotenv is loaded before imported options
option("papyrus_path")
    add_deps("dotenv")
option("spriggit_path")
    add_deps("dotenv")

-- hide commonlibsse-ng options
option("rex_ini")
    set_showmenu(false)
option("rex_json")
    set_showmenu(false)
option("rex_toml")
    set_showmenu(false)
option("skse_xbyak")
    set_showmenu(false)
option("tests")
    set_showmenu(false)
option_end()

-- Dependencies & Includes
-- https://github.com/xmake-io/xmake-repo/tree/dev
add_requires("yaml-cpp", "magic_enum", "nlohmann_json", "simpleini", "glm")

-- policies
set_policy("package.requires_lock", true)

-- rules
add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    add_defines("DEBUG")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("fastest")
    set_symbols("debug")
end

set_allowedplats("windows")
set_allowedarchs("x64")
set_defaultplat("windows")
set_defaultarchs("x64")

--Applied to ALL targets
rule("common")
    after_config(function(target)
        -- after_config to ensure install_path overwrites value set by
        -- commonlibsse-ng.plugin
        import("core.project.config")
        local install_path = config.get("install_path")
        if install_path then
            target:set("installdir", install_path, {readonly = true})
        end
    end)

    after_build(function(target)
        import("core.project.config")
        import("core.base.task")
        local auto_install = config.get("auto_install")
        local install_path = config.get("install_path")
        if auto_install and install_path and target:name() ~= "commonlibsse-ng" then
            task.run("install", {target = target:name()})
        end
    end)

    on_install(function(target)
        local srcfiles, dstfiles = target:installfiles()
        if srcfiles and dstfiles then
            for idx, srcfile in ipairs(srcfiles) do
                os.vcp(srcfile, dstfiles[idx], {copy_if_different = true})
            end
        end
    end)
rule_end()
add_rules("common")

-- Target
target(PROJECT_NAME)
    set_enabled(get_config("build_dll"))
    -- Dependencies
    add_packages("yaml-cpp", "magic_enum", "nlohmann_json", "simpleini", "glm")

    -- CommonLibSSE
    add_deps("commonlibsse-ng")
    add_rules("commonlibsse-ng.plugin", {
        name = PROJECT_NAME,
        author = "Scrab",
        description = "Backend for skyrims adult animation framework 'SexLab'."
        })

    -- Source files
    set_pcxxheader("src/PCH.h")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")

    -- flags
    add_cxxflags(
        "cl::/cgthreads4",
        "cl::/diagnostics:caret",
        "cl::/external:W0",
        "cl::/fp:contract",
        "cl::/fp:except-",
        "cl::/guard:cf-",
        "cl::/Zc:enumTypes",
        "cl::/Zc:preprocessor",
        "cl::/Zc:templateScope",
        "cl::/utf-8"
        )
    -- flags (cl: warnings -> errors)
    add_cxxflags("cl::/we4715") -- `function` : not all control paths return a value
    -- flags (cl: disable warnings)
    add_cxxflags(
        "cl::/wd4068", -- unknown pragma 'clang'
        "cl::/wd4201", -- nonstandard extension used : nameless struct/union
        "cl::/wd4265" -- 'type': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
        )
    -- Conditional flags
    if is_mode("debug") then
        add_cxxflags("cl::/bigobj")
    elseif is_mode("release") then
        add_cxxflags("cl::/Zc:inline", "cl::/JMC-", "cl::/Ob3")
    end
    on_load(function(target)
        local clib = target:rule("commonlibsse-ng.plugin")
        if clib then
            -- disable unwanted events
            clib:set("install", nil)
            clib:set("package", nil)
            clib:set("build_after", nil)
        end
    end)
    -- Post Build 
    after_build(function (target)
        import("lib.detect.find_tool")
        local python = find_tool("python3")
        if python then
            os.execv(python.program, {"scripts/generate_config.py"})
        else
            print("Warning: Python not found. Skipping config generation.")
        end

        os.vcp(target:targetfile(), "dist/SKSE/Plugins")
        os.vcp(target:symbolfile(), "dist/SKSE/Plugins")
        os.vcp("scripts/out/*", "dist/SKSE/Plugins", {copy_if_different = true})
        target:add("installfiles", "scripts/out/SexLab.ini", {prefixdir = "SKSE/Plugins"})
    end)
target_end()

target("papyrus")
    set_enabled(get_config("build_papyrus"))
    set_kind("object")
    set_targetdir("dist/Scripts")
    set_basename("SexLab")

    --avoid rebuild on mode change
    set_policy("build.intermediate_directory", false)

    add_rules("papyrus")

    add_files("dist/Source/Scripts/*.psc")

    add_includedirs("dist/Source/Scripts")
    add_includedirs("$(papyrus_include)/SkyrimLovense/Source/Scripts")
    add_includedirs("$(papyrus_include)/PapyrusUtil SE - Modders Scripting Utility Functions/Source/Scripts")
    add_includedirs("$(papyrus_include)/SkyUI SDK/Source/Scripts")
    add_includedirs("$(papyrus_include)/Race Menu Sources/Source/Scripts")
    add_includedirs("$(papyrus_include)/MfgFix NG/Source/Scripts")
    add_includedirs("$(papyrus_include)/AnimSpeedSE/Source/Scripts")
    add_includedirs("$(papyrus_gamesource)/Source/Scripts")

    on_load(function(target)
        import("core.project.config")

        if not config.get("papyrus_include") then
            cprint("${color.warning}papyrus_include is not defined")
        end
        if not config.get("papyrus_gamesource") then
            cprint("${color.warning}papyrus_gamesource is not defined")
        end
    end)
    before_build(function(target)
        import("core.project.config")
        assert(config.get("papyrus_include"), "papyrus_include is not defined")
        assert(config.get("papyrus_gamesource"), "papyrus_gamesource is not defined")
    end)
target_end()

target("SexLab.esm")
    set_enabled(get_config("build_spriggit"))
    set_kind("object")
    set_targetdir("dist")

    --avoid rebuild on mode change
    set_policy("build.intermediate_directory", false)

    add_rules("spriggit")
    set_values("spriggit.srcdir", "res/SexLab.esm")
target_end()

target("assets")
    set_enabled(get_config("build_assets"))
    set_kind("phony")

    add_installfiles("dist/(Interface/SexLab/**)")
    add_installfiles("dist/(Interface/Translations/*.txt)")
    add_installfiles("dist/(SKSE/CustomConsole/*.yaml)")
    add_installfiles("dist/(SKSE/SexLab/**)")

    add_installfiles("dist/(meshes/**)")
    add_installfiles("dist/(textures/**)")
    add_installfiles("dist/(Sound/**)")
target_end()

task("serialize")
    on_run(function()
        import("core.base.option")
        import("core.base.task")
        task.run("config")
        task.run("spriggit.serialize", {
            target = "SexLab.esm",
            install = option.get("install"),
        })
    end)

    set_menu {
        shortname = 's',
        usage = "xmake serialize [-i | <srcfile>]",
        description = "Serialize SexLab.esm",
        options = {
            {'i', "install",    "k",  nil, "Serialize from installdir." },
            {'o', "installdir", "kv", nil, "Override the install directory."},
        }
    }
task_end()

includes("@builtin/xpack")

xpack("SexLab")
    set_formats("zip")
    set_extension(".7z")
    set_basename("SexLab Framework PPLUS - V$(version)")
    add_targets(PROJECT_NAME, "papyrus", "assets", "SexLab.esm")
    on_installcmd(function(package, batchcmds)
        local installdir = package:installdir()
        local install = import("test_ln")() and batchcmds.ln or batchcmds.cp
        for _, v in pairs(package:targets()) do
            srcfiles, dstfiles = v:installfiles(installdir)
            for idx, srcfile in ipairs(srcfiles) do
                install(batchcmds, path.absolute(srcfile), dstfiles[idx])
            end
        end
    end)
