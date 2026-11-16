@echo off
rem Build OpenHydroQual for Windows and package the installer.
rem Run from anywhere; paths are relative to this script.
rem QTDIR selects the Qt kit (default below). It must match <QtInstall> in QAquifolium.vcxproj.
setlocal
cd /d "%~dp0.."

if "%QTDIR%"=="" set "QTDIR=C:\Qt\6.7.2\msvc2019_64"
set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
set "ADVINST=C:\Program Files (x86)\Caphyon\Advanced Installer 15.1\bin\x86\AdvancedInstaller.com"

echo === Building Release^|x64
"%MSBUILD%" OpenHydroQual.sln /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal || goto :fail

echo === Deploying Qt from %QTDIR%
for %%E in (OpenHydroQual.exe ModelWizard.exe OHQTerminal.exe) do (
    "%QTDIR%\bin\windeployqt.exe" --release --no-translations "x64\Release\%%E" || goto :fail
)

echo === Checking templates, icons and installer files
python tools\check_icons.py || goto :fail
python tools\check_installer.py || goto :fail

echo === Building the installer
"%ADVINST%" /build Setup\QAquifolium.aip || goto :fail

echo === Done
exit /b 0

:fail
echo *** Release preparation failed; the installer was not built.
exit /b 1
