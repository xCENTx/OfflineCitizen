# Building OfflineCitizen

Use Visual Studio 2026 or its Build Tools with the Desktop development with C++ components, the v145 C++ toolset and a Windows SDK.

1. Open `OfflineCitizen.sln`.
2. Select `Release` and `x64` (the game and SDK are 64-bit).
3. Build the solution. Both projects write to `bin/x64/Release/`:
   - `SoloCitizen.dll`
   - `StarCitizen_Launcher.exe`

From a Visual Studio developer terminal, the equivalent command is:

```text
msbuild OfflineCitizen.sln /m /p:Configuration=Release /p:Platform=x64
```

The Debug/x64 configuration is also supported. Existing SDK/compiler warnings and missing third-party debug-symbol warnings may appear.

## Local launcher configuration

The launcher reads a local JSON file. It does not fetch configuration.

1. Copy `launcher/launch-settings.example.json` to `launch-settings.json` beside the launcher.
2. Replace the example `parameters` value with the launch-argument string appropriate to your installed game version. The example is intentionally not a working game configuration; no personal arguments or account values are bundled.
3. Place the launcher and `SoloCitizen.dll` beside `StarCitizen.exe`, and use that directory as the working directory. Alternatively, set the three local path defaults in `launcher/launcher-config.h` and rebuild.

`launcher-config.h` defines the JSON path, executable path and DLL path. Keep any personal `launch-settings.json` out of uploaded archives.

The game hooks and offsets are version-specific. A successful build does not establish compatibility with a different game build. Runtime behavior must be checked against your own matching installation.

## Package contents

The source archive includes the project, source, dependencies and public configuration example. It contains no Git history or personal settings. The separate Release archive contains only the compiled launcher, DLL, configuration example, instructions and license.
