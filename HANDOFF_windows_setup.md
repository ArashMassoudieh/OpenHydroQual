# Windows build & installer handoff — OpenHydroQual 2.0.7

Target: build the GUI with **Visual Studio**, package with **Advanced Installer**,
and attach the installer to the existing GitHub release `v2.0.7` alongside the
Linux `.deb`.

Written from the Linux side on 2026-09-06 at commit `e24ea5e`. Everything below
was verified against the repository, not recalled. Where something is unverified
it says so explicitly.

---

## 0. The one thing that will silently break the install

`mainwindow.h:41`

```cpp
#ifdef windows_version
    #define RESOURCE_DIRECTORY qApp->applicationDirPath().toStdString()+"/../../resources"
#endif
```

On Windows the application looks for its templates **two directory levels above
the executable**. `resources/` holds `main_components.json`, `settings.json` and
every plugin template; without it the app starts but cannot build a model, and
the failure looks like an empty component palette rather than a missing-file
error.

**The installed layout must therefore be:**

```
<InstallDir>\
    resources\                       <- the whole resources tree
        main_components.json
        settings.json
        Icons\
        ...
    bin\
        Release\
            OpenHydroQual.exe        <- exe two levels below <InstallDir>
            Qt6*.dll, platforms\, styles\, ...
```

This mirrors the Debian package exactly (`/opt/OpenHydroQual/bin/Release/` with
`/opt/OpenHydroQual/resources/`), which is the layout the macro was written for.

**Do not** let Advanced Installer place `OpenHydroQual.exe` at the install root.
It is the obvious thing to do and it will break resource resolution. The `bin\Release`
nesting is load-bearing, not cosmetic.

Verify after installing: launch the app and confirm the block/link palette is
populated with icons. An empty palette means `RESOURCE_DIRECTORY` did not resolve.

---

## 1. Prerequisites

| Item | Detail |
|---|---|
| Qt | **Qt 6**, MSVC 64-bit kit. Modules required: `core gui opengl printsupport widgets svgwidgets charts`. Qt 5 is still supported by the `.pro` (`svg charts` instead) but 2.0.7 is developed against Qt 6. |
| Compiler | MSVC x64. `CONFIG += c++17`. |
| qcustomplot6 | **Sibling directory**, not vendored: `OpenHydroQual.pro:38` has `INCLUDEPATH += ../qcustomplot6/`. Clone/copy it next to the OpenHydroQual checkout, i.e. `<parent>\qcustomplot6\`. |
| jsoncpp | **In-tree, already compiled by the .pro** (`jsoncpp/src/lib_json/*.cpp`). No external package needed. |
| Armadillo | **Vendored in `include/`** (version 6.100.0). Windows reaches it via `LAPACK_INCLUDE = $$PWD/include` inside the `win32` block. See §3 — do not "upgrade" it casually. |
| LAPACK/BLAS | **MISSING from the repository.** See §2. This is the blocker. |
| GSL | Probably not needed — see §4. |

---

## 2. BLOCKER: the win64 LAPACK/BLAS libraries are not in the repo

`OpenHydroQual.pro` win32 block expects:

```
libs/lapack-blas_lib_win64/debug/    lapack_win64_MTd.lib, blas_win64_MTd.lib
libs/lapack-blas_lib_win64/release/  lapack_win64_MT.lib,  blas_win64_MT.lib
```

`libs/` **does not exist** in the checkout and is not tracked in git. The Windows
build cannot link until these are supplied. Options, in order of preference:

1. **Locate the original libraries.** They were evidently present on the machine
   that last built for Windows. Check that machine before rebuilding them.
2. Download prebuilt LAPACK/BLAS win64 static libs built with the `/MT` runtime
   (the `_MT` / `_MTd` suffixes in the filenames say the multithreaded static CRT
   is expected — the VS project must use `/MT` and `/MTd` to match, or you will
   get CRT mismatch link errors).
3. Build LAPACK/BLAS from source with CMake + Intel Fortran or flang.

If you place them elsewhere, change `LAPACK_LIB_DIR` in the `win32` block rather
than hardcoding a path anywhere else.

**Ask Arash before spending long on option 3** — option 1 is very likely.

---

## 3. Armadillo version: a trap that already caused one round of breakage

`include/` contains **Armadillo 6.100.0**, bundled for Windows. Linux and macOS
now use the system Armadillo (12.x). The code carries version guards because of
this split:

- `aquifolium/src/System.cpp:5916` — `#if defined(ARMA_VERSION_MAJOR) && (ARMA_VERSION_MAJOR >= 8)` around `superlu_opts`, which does not exist in Armadillo 6.
- `aquifolium/src/Matrix_arma.cpp` — `Invert()` uses a hand-rolled infinity norm rather than `arma::rcond`, which Armadillo 6 lacks.

If you replace the vendored Armadillo with a newer one, those guards will start
taking their modern branches and the Windows build will compile a different code
path than it has historically. That may be fine, but it is a deliberate change,
not a free upgrade. **Default position: leave `include/` alone.**

Note: on 2026-09-06 the unguarded `INCLUDEPATH += include/` was moved out of the
global section into `win32` only (it was silently forcing Linux/macOS onto
Armadillo 6). Windows still gets it via `INCLUDEPATH += $${LAPACK_INCLUDE}` at
`OpenHydroQual.pro:320`. Both paths resolve to the same `include/` directory.

---

## 4. GSL — verify at link time, probably not needed

`OpenHydroQual.pro:45` sets `DEFINES += GSL` globally, and `aquifolium/include/BTC.h`
includes `<gsl/gsl_rng.h>` unconditionally. However **the GUI `.pro` does not
compile `BTC.cpp` and no GUI source appears to include `BTC.h`**, so GSL is very
likely not required for this target. The Linux block links `-lgsl` regardless.

If the linker reports unresolved `gsl_*` symbols, install GSL for MSVC (vcpkg:
`vcpkg install gsl:x64-windows`) and add it to the `win32` block. Otherwise
ignore it. Do not pre-emptively add GSL.

---

## 5. Version 2.0.7 — already set, do not bump

Consistent across:

- `mainwindow.cpp:17` — `#define openhydroqual_version "2.0.7"`
- `OHQLib/OHQLib.pro:10` — `VERSION = 2.0.7`
- `PythonBindings/PythonBindings.pro:8` — `VERSION = 2.0.7`
- `build_OHQ_and_push.sh` — `VERSION="2.0.7"`

The published release is currently v2.0.6; v2.0.7 is being cut now. Use **2.0.7**
in the Advanced Installer product version so the installer and the `.deb` agree.

---

## 6. Deployment (windeployqt)

After a successful Release build, from a Qt/MSVC developer prompt:

```
windeployqt --release --no-translations <path>\bin\Release\OpenHydroQual.exe
```

This pulls in the Qt DLLs plus the `platforms\`, `styles\`, `imageformats\`
plugin folders. Confirm `Qt6Charts.dll` and `Qt6SvgWidgets.dll` are among them —
those come from the Qt-6-specific `QT +=` line and are easy to miss if the app is
tested only on a machine with Qt installed.

Also ship the MSVC redistributable, or link the static CRT (`/MT`) consistently
with the LAPACK libs from §2.

---

## 7. What goes in the installer

`resources/` is **92 MB**, dominated by:

| | size |
|---|---|
| `resources/Icons` | 46 MB |
| `resources/Wizard_Scripts` | 26 MB |
| `resources/Wizard_Scripts_server` | 20 MB |
| everything else (the JSON templates) | ~1 MB |

`Icons` is required — the palette uses it (`gridgenerator.cpp:80`). The JSON
templates are required. **`Wizard_Scripts_server` is very likely not needed in a
desktop installer** (the name suggests the server deployment), which would cut
20 MB. Confirm with Arash before excluding it; the Debian package currently ships
everything.

---

## 8. Known build hazards

**Stale generated `ui_*.h` files.** An in-source Qt build leaves `ui_*.h` at the
repo root. They are gitignored, and a set of old ones is quarantined in
`.stale_insource_build/` — do not move those back. If you get errors like
`cannot convert 'logwindow*' to 'QDialog*'`, a stale `ui_logwindow.h` is being
picked up ahead of the freshly generated one. Delete every `ui_*.h` in the repo
root and in the build directory, then rebuild. This exact error cost a debugging
round on Linux.

**`#ifdef DEBUG` remaps matrix types.** `aquifolium/include/System.h:54`:

```cpp
#ifdef DEBUG
#define CVector_arma CVector
#define CMatrix_arma CMatrix
#endif
```

A Debug build therefore compiles genuinely different linear-algebra types than
Release. Behaviour that differs between Debug and Release is expected here, not a
bug. **Package the Release build.**

**Do not mix Qt installations.** Use one kit consistently. On Linux, running a
different qmake inside an existing build directory corrupted it and required a
full regeneration.

---

## 9. Publishing to GitHub

The release `v2.0.7` will already exist (created by the Linux script
`build_OHQ_and_push.sh`, run from `/home/arash/Projects`, which uploads
`openhydroqual_amd64.deb`). Attach the Windows installer to the same release:

```
gh release upload v2.0.7 OpenHydroQual_2.0.7_setup.exe --repo ArashMassoudieh/OpenHydroQual --clobber
```

Keep the asset filename **stable across releases** if a `latest` download URL is
wanted for Windows, mirroring the Linux one:

```
https://github.com/ArashMassoudieh/OpenHydroQual/releases/latest/download/openhydroqual_amd64.deb
```

so the Windows equivalent would be, with a fixed name such as
`openhydroqual_setup.exe`:

```
https://github.com/ArashMassoudieh/OpenHydroQual/releases/latest/download/openhydroqual_setup.exe
```

A versioned filename breaks that URL. Decide before publishing.

---

## 10. Verification checklist

- [ ] Release build links (no unresolved LAPACK/BLAS, no CRT mismatch)
- [ ] `windeployqt` run; `Qt6Charts.dll` and `Qt6SvgWidgets.dll` present
- [ ] Installed layout is `<InstallDir>\bin\Release\OpenHydroQual.exe` with `<InstallDir>\resources\`
- [ ] App launches on a machine **without** Qt installed
- [ ] **Component palette is populated with icons** — the resource-path check
- [ ] Open a model and run it: `File > Open` one of the `.ohq` examples, then Run
- [ ] Help/About reports version 2.0.7
- [ ] Installer uploaded to release `v2.0.7`

---

## Open questions for Arash

1. Where are the win64 LAPACK/BLAS `.lib` files? (§2 — the blocker)
2. Should `Wizard_Scripts_server` ship in the desktop installer? (§7 — 20 MB)
3. Fixed installer filename for a `latest` URL, or versioned? (§9)
