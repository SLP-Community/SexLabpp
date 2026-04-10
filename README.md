# SexLab P+
A high performance and stability patch for SexLab for Skyrim SE.

## Requirements
* [python](https://www.python.org/downloads/)
* [xmake](https://xmake.io/#/)
	* Add this to your `PATH`
* [Spriggit CLI](https://github.com/Mutagen-Modding/Spriggit/)
	* Requires [Microsoft .NET SDK](https://dotnet.microsoft.com/en-us/download/dotnet/)
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++
	* On VS Community 2026, use this command first:
		```sh
		xmake f --cxflags="/Wv:18"
		```
* Papyrus Sources:
	* PapyrusUtil SE: [Nexus][PU-Nexus] | [GitHub][PU-GitHub]
	* SkyUI SDK 5.1: [GitHub][SUI-GitHub]
	* Race Menu (Modders Package): [Nexus][RM-Nexus]
	* MfgFix NG: [Nexus][MFG-Nexus] | [GitHub][MFG-GitHub]
	* SkyrimLovense: [Nexus][SL-Nexus] | [GitHub][SL-GitHub]
	* VRIK Player Avatar: [Nexus][VR-Nexus]

## Building

### Clone

```sh
git clone https://github.com/Scrabx3/SexLabpp.git
cd SexLabpp
git submodule update --init --recursive
```

### Configure
Before building you must provide the following options: **spriggit_path**, **papyrus_path**, **papyrus_include**, and **papyrus_gamesource**. These can be provided using either:

1. Environment variables (optionally loaded from `.env`)
   - See `.env.example` for variables/descriptions.
   - Copy to `.env` and edit the values.
   - Run `xmake f` after making changes to load the new values.
   - Alternate files can be loaded with `xmake f --dotenv=.env.other`

2. Command-line configure
   - Set options when configuring with `xmake f`:
```sh
xmake f -m release \
	--spriggit_path="C:\path\to\Spriggit.CLI" \
	--papyrus_path="C:\path\to\Papyrus Compiler" \
	--papyrus_include="C:\path\to\ModOrganizer\mods" \
	--papyrus_gamesource="C:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition\Data"
```

### Build

```sh
# Everything
xmake
# SexLabUtil.dll
xmake build SexLabUtil
# Papyrus Scripts
xmake build papyrus
# SexLab.exm
xmake build SexLab.esm
```

### Install

If `install_path` and `auto_install` are configured, files will be automatically coppied to `install_path` after a successful build. Otherwise install can be run manually using:
```sh
xmake install -o INSTALLDIR
```

## Plugin Serialization

After making changes to SexLab.esm, serialize back to yaml using:

```sh
# From .\dist\SexLab.esm
xmake serialize
# From install_path
xmake serialize -i
```

## Papyrus Project Generation

Generate a papyrus project file for IDE integration using:
```sh
xmake papyrus.project papyrus
```

## License

This project primarily falls under the [Apache License Version 2.0](./LICENSE). However, portions of the project derived from the original SexLab (including papyrus scripts, and various assets) remain subject to the Permissions terms found in [Readme - SexLabFramework.txt](./Readme%20-%20SexLab%20Framework.txt).


[PU-Nexus]: https://www.nexusmods.com/skyrimspecialedition/mods/13048
[PU-GitHub]: https://github.com/eeveelo/PapyrusUtil
[SUI-GitHub]: https://github.com/schlangster/skyui/wiki
[RM-Nexus]: https://www.nexusmods.com/skyrimspecialedition/mods/19080?tab=files&file_id=63518
[MFG-Nexus]: https://www.nexusmods.com/skyrimspecialedition/mods/133568
[MFG-GitHub]: https://github.com/KrisV-777/Mfg-Fix-NG
[SL-Nexus]: https://www.nexusmods.com/skyrimspecialedition/mods/133698
[SL-GitHub]: https://github.com/KrisV-777/Skyrim-Lovense
[UIX-Nexus]: https://www.nexusmods.com/skyrimspecialedition/mods/17561
[BAE-Nexus]: https://www.nexusmods.com/skyrimspecialedition/mods/974
[VR-Nexus]: https://www.nexusmods.com/skyrimspecialedition/mods/23416
