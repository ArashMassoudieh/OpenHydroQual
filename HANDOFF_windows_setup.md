# Windows build & installer handoff: OpenHydroQual 2.1.1

Target: build the GUI with **Visual Studio** (`QAquifolium.vcxproj`), package it
with **Advanced Installer** (`Setup/QAquifolium.aip`), and attach the installer
to the existing GitHub release `v2.1.1` next to the Linux `.deb`.

Written from the Linux side on 2026-10-13 at commit `39c0fa4`. Everything below
was checked against the repository, not recalled. Anything that could not be
checked from Linux says so.

**Since the 2.0.7 handoff:** the Windows build is done with the Visual Studio
project and Advanced Installer, not with qmake. The VS project and the `.aip`
were last updated for 2.1.1 on 2026-09-27 (`f54ac28`), but 2.1.1 work continued
after that, and three things need fixing before the installer will work (§2).

---

## 0. The one thing that will silently break the install

`mainwindow.h:42`

```cpp
#ifdef windows_version
    #define RESOURCE_DIRECTORY qApp->applicationDirPath().toStdString()+"/../../resources"
#endif
```

On Windows the application looks for its templates **two directory levels above
the executable**. `resources/` holds `main_components.json`, `settings.json` and
every plugin template; without it the app starts but cannot build a model, and
the failure looks like an empty component palette, with no missing-file error.

The `.aip` already gets this right:

```
<InstallDir>\
    resources\                 <- templates, Icons\, Wizard_Scripts\
    bin\
        bin\
            OpenHydroQual.exe  <- two levels below <InstallDir>
            Qt6*.dll, platforms\, gsl.dll, libopenblas.dll, ...
```

Keep it that way. **Do not** move `OpenHydroQual.exe` to the install root or to
a single `bin\`.

Verify after installing: launch the app and confirm the block/link palette is
populated with icons. An empty palette means `RESOURCE_DIRECTORY` did not
resolve.

---

## 1. Build configuration (Release|x64 in `QAquifolium.vcxproj`)

| Item | Detail |
|---|---|
| Qt | **Qt 6.7** (`<QtInstall>6.7</QtInstall>`; the `.aip` takes plugin DLLs from `Qt\6.7.2\msvc2019_64`). Modules: `core gui widgets opengl printsupport svg svgwidgets charts`. **`concurrent` is missing, see §2.1.** |
| Compiler | MSVC x64, `stdcpp17`, runtime `/MD` (MultiThreadedDLL). |
| Defines | `windows_version`, `Qt6`, `GSL`, `QCharts`, `ARMA_USE_LAPACK`, `ARMA_USE_BLAS`, `ARMA_DONT_USE_WRAPPER`, `NDEBUG`, among others. |
| Armadillo | `C:\Projects\armadillo-15.2.3\include`. The VS build does **not** use the vendored Armadillo 6.100 in `include/`; that copy is only used by the qmake `win32` block. |
| BLAS/LAPACK | OpenBLAS 0.3.31 at `C:\Projects\OpenBLAS-0.3.31-x64\win64`, linked as `libopenblas.lib`. |
| GSL | **Required.** vcpkg package at `..\vcpkg\packages\gsl_x64-windows` (a sibling of the checkout), linked as `gsl.lib` and `gslcblas.lib`. `TimeSeries.h` includes GSL headers directly. |
| jsoncpp | In-tree (`jsoncpp\include`). |
| codegen | `.\codegen\include` is on the include path and the codegen sources are in the project, so **Export to C++** is part of the GUI. |

The `.aip` also expects `libs\gsl\` and `libs\lapack-blas_lib_win64\` (holding
`gsl.dll`, `gslcblas.dll`, `blas_win64_MT.dll`, `lapack_win64_MT.dll`) next to
`Setup\`. `libs/` is not tracked in git; it exists only on the Windows build
machine.

---

## 2. Required fixes before building 2.1.1

### 2.1 Add Qt Concurrent

`mainwindow.cpp:95` includes `<QtConcurrent/QtConcurrent>` and
`mainwindow.cpp:646` calls `QtConcurrent::run` (used by the C++ export
validation). `OpenHydroQual.pro:11` has `QT += ... concurrent`, but the VS
project does not.

- Add `concurrent` to `<QtModules>` for Release|x64 in `QAquifolium.vcxproj`.
- Add `Qt6Concurrent.dll` to the installer next to the other Qt DLLs. It is not
  in the `.aip`, so even a successful build would fail to start on a machine
  without Qt.

### 2.2 Add the missing resource files to the installer

The `.aip` lists every resource file individually, so files added since it was
last edited are not installed. These templates are **in
`resources/default_templates.list`**, which the app loads at startup, but are
**missing from the `.aip`**:

- `snowmelt.json`
- `urban_hru.json`
- `riparian_urban_hru.json`
- `dual_drainage.json`
- `physicsbaseddatadrivenhydrology.json`

Also missing (templates that models can require, and their icons):

- `compound_channel.json`, `dam_spillways.json`, `floodplain_bypass.json`,
  `rainfall_runoff_depthonly.json`, `unsaturated_soil_revised_model_EC_logistic.json`,
  `Upscaling.json`
- Icons: `snowmelt.png`, `urban_hru.png`, `dual_drainage_inlet.png`,
  `dual_drainage_outfall.png`, `HRU_Pervious_Outlet.png`,
  `HRU_Impervious_Outlet.png`, `channel2soil.png`, `channel2aggregate.png`,
  `Compound_Channel.png`, `Compound_Channel_Link.png`, `Emergency_Spillway.png`,
  `Riser_Spillway.png`, `Floodplain_Bypass_Link.png`,
  `reaeration_Banks_Herrera.png`, `reaeration_Churchill.png`,
  `reaeration_OConnor_Dobbins.png`, `reaeration_Owens_Gibbs.png`
- 42 files under `resources/Wizard_Scripts/`

The simplest fix is to add `resources\` as a synchronized folder in Advanced
Installer, so new files are picked up automatically, and exclude anything that
should not ship (for example `.Rhistory`, `*.bak*`). Otherwise add the files
above by hand.

### 2.3 Check two sources missing from the VS project

`statusviewer.cpp` and `XString.cpp` are in `OpenHydroQual.pro` but not in
`QAquifolium.vcxproj`. Both are referenced by GUI code. If the link reports
unresolved symbols from them, add them to the project. This has not been
checked on Windows.

---

## 3. Version 2.1.1: already set, do not bump

Consistent across:

- `mainwindow.cpp:17`: `#define openhydroqual_version "2.1.1"`
- `OHQLib/OHQLib.pro:10`: `VERSION = 2.1.1`
- `PythonBindings/PythonBindings.pro:8`: `VERSION = 2.1.1`
- `build_OHQ_and_push.sh`: `VERSION="2.1.1"`
- `Setup/QAquifolium.aip`: `ProductVersion` `2.1.1`

The `.aip` ProductCode was already regenerated for 2.1.1. Do not change it again
unless the version changes.

---

## 4. Deployment (windeployqt)

After a successful Release build, from a Qt/MSVC developer prompt:

```
windeployqt --release --no-translations x64\Release\OpenHydroQual.exe
```

The `.aip` takes most Qt DLLs from `x64\Release\`, where windeployqt puts them,
and `Qt6OpenGL.dll`, `Qt6OpenGLWidgets.dll` and `qwindows.dll` from the Qt 6.7.2
installation. Confirm `Qt6Charts.dll`, `Qt6SvgWidgets.dll` and (after §2.1)
`Qt6Concurrent.dll` are present.

The build uses the DLL runtime (`/MD`), so the MSVC redistributable must be
installed on the target machine. Check that the `.aip` includes it as a
prerequisite; this could not be confirmed from Linux.

---

## 5. What goes in the installer

`resources/` is about 95 MB:

| | size |
|---|---|
| `resources/Icons` | 48 MB |
| `resources/Wizard_Scripts` | 26 MB |
| `resources/Wizard_Scripts_server` | 20 MB |
| everything else (the JSON templates) | ~1 MB |

The installer currently ships `Icons`, the JSON templates and `Wizard_Scripts`,
and leaves out `Wizard_Scripts_server`. If you switch to a synchronized folder
(§2.2), exclude `Wizard_Scripts_server` to keep it that way.

Besides `OpenHydroQual.exe`, the installer ships `ModelWizard.exe`,
`OHQTerminal.exe` and `updater.exe`. Rebuild those too if their sources changed.

---

## 6. Known build hazards

**Stale generated `ui_*.h` files.** An in-source Qt build leaves `ui_*.h` at the
repo root. If you get errors like `cannot convert 'logwindow*' to 'QDialog*'`, a
stale `ui_logwindow.h` is being picked up ahead of the freshly generated one.
Delete every `ui_*.h` in the repo root and in the build directory, then rebuild.

**`#ifdef DEBUG` remaps matrix types.** `aquifolium/include/System.h:54`:

```cpp
#ifdef DEBUG
#define CVector_arma CVector
#define CMatrix_arma CMatrix
#endif
```

A Debug build therefore compiles different linear-algebra types from Release.
Behaviour that differs between Debug and Release is expected here. **Package
the Release build.**

**Armadillo version.** `aquifolium/src/System.cpp:6594` guards `superlu_opts`
with `ARMA_VERSION_MAJOR >= 8`. With Armadillo 15 the VS build takes the modern
branch, which matches Linux and macOS. Do not point the VS project back at the
vendored Armadillo 6 in `include/`.

**Do not mix Qt installations.** The project and the `.aip` both use Qt 6.7;
keep one kit.

---

## 7. Publishing to GitHub

Release `v2.1.1` already exists and currently holds only
`openhydroqual_amd64.deb`. The `.aip` writes the package as `OpenHydroQual` in
`..\..\..\Dropbox\Aquifolium` (relative to `Setup\`); check the actual output
file name and extension after building. Attach it to the same release:

```
gh release upload v2.1.1 <installer file> --repo ArashMassoudieh/OpenHydroQual --clobber
```

Keep the asset name **the same across releases** if a `latest` download URL is
wanted for Windows, mirroring the Linux one:

```
https://github.com/ArashMassoudieh/OpenHydroQual/releases/latest/download/openhydroqual_amd64.deb
```

With a fixed name such as `openhydroqual_setup.exe` (or `.msi`), the Windows URL
would be:

```
https://github.com/ArashMassoudieh/OpenHydroQual/releases/latest/download/openhydroqual_setup.exe
```

A versioned file name breaks that URL. Rename the file before uploading if
needed.

---

## 8. Verification checklist

- [ ] `concurrent` added to the VS project; `Qt6Concurrent.dll` added to the `.aip` (§2.1)
- [ ] Missing templates, icons and wizard scripts added to the `.aip` (§2.2)
- [ ] Release|x64 build links (check `statusviewer.cpp` and `XString.cpp`, §2.3)
- [ ] `windeployqt` run; `Qt6Charts.dll`, `Qt6SvgWidgets.dll`, `Qt6Concurrent.dll` present
- [ ] Installed layout is `<InstallDir>\bin\bin\OpenHydroQual.exe` with `<InstallDir>\resources\`
- [ ] App launches on a machine **without** Qt or Visual Studio installed
- [ ] **Component palette is populated with icons**, including Snow, Urban HRU, Riparian Urban HRU and Dual Drainage
- [ ] Open one of the `.ohq` examples (for example `Examples\Snowmelt`) and run it
- [ ] Help/About reports version 2.1.1
- [ ] Installer uploaded to release `v2.1.1`

---

## Open questions for Arash

1. Switch `resources\` to a synchronized folder in the `.aip`, or keep listing files by hand? (§2.2)
2. Fixed installer file name for a `latest` URL, or versioned? (§7)
3. Is the MSVC redistributable included as a prerequisite in the `.aip`? (§4)
