@echo off
cd %cd%
: Running CMake
cmd /c "cmake" -S %cd%/ -B %cd%/Build
: start %cd%/Build/LightingEffects.sln
: cmd /c "vcpkg"
echo.

: Creating shortcut (*A little ugly, but get the work done)
echo Creating shortcut to %cd%/LightingEffects.sln...
echo.
echo Set fs = CreateObject("Scripting.FileSystemObject") >> CreateShortcut.vbs
echo Set oWS = WScript.CreateObject("WScript.Shell") >> CreateShortcut.vbs
echo sLinkFile = "LightingEffects.lnk" >> CreateShortcut.vbs
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> CreateShortcut.vbs
echo oLink.TargetPath = fs.BuildPath(oWS.CurrentDirectory, "Build/LightingEffects.sln") >> CreateShortcut.vbs
echo oLink.Save >> CreateShortcut.vbs
cscript CreateShortcut.vbs
del CreateShortcut.vbs

pause
