# OpenHydroQual

**OpenHydroQual** is an open-source environmental simulation platform built on the *aquifolium* engine. It models water flow, solute transport, and biogeochemical processes in interconnected hydrological systems using a flexible block-and-link graph structure.

The platform supports groundwater, surface water, wastewater, and coupled multi-domain models. It ships with Python bindings (`openhydroqual_py`) for programmatic model building, calibration workflows, and reinforcement learning.

---

## Repository Structure

```
OpenHydroQual/
├── aquifolium/              Core simulation engine (C++)
│   ├── src/                 Source files
│   └── include/             Public headers
├── OHQLib/                  Shared library project
│   ├── OHQLib.pro           qmake build (Linux/macOS, QtCreator)
│   └── CMakeLists.txt       CMake build (cross-platform, MSVC)
├── PythonBindings/          Python bindings (pybind11)
│   ├── PythonBindings.pro   qmake build
│   └── bindings.cpp         pybind11 binding definitions
├── jsoncpp/                 JSON library (git submodule)
├── libs/
│   └── lapack-blas_lib_win64/  Pre-built LAPACK/BLAS for Windows
├── resources/               JSON component template files
│   ├── main_components.json
│   ├── settings.json
│   └── unconfined_groundwater.json
└── XString.h                Shared string utility header
```

---

## Prerequisites

### All Platforms
| Dependency | Version | Purpose |
|---|---|---|
| Qt | 6.x | Core JSON, file I/O, containers |
| CMake | ≥ 3.20 | Cross-platform build (Windows / CI) |
| Armadillo | any recent | Linear algebra |
| LAPACK + BLAS | any | Matrix solvers |

### Linux (Ubuntu/Debian)
| Dependency | Version | Purpose |
|---|---|---|
| GCC or Clang | ≥ 9 | C++14 compiler |
| OpenMP | — | Parallel solving |
| GSL | — | Scientific functions |

### macOS
| Dependency | Version | Purpose |
|---|---|---|
| Xcode Command Line Tools | — | Compiler |
| Homebrew | — | Package manager |
| libomp | — | OpenMP runtime |

### Windows
| Dependency | Version | Purpose |
|---|---|---|
| Visual Studio | 2019 or 2022 | MSVC compiler |
| Qt for MSVC | 6.x | Core library |

---

## Quick Start

```bash
# 1. Clone with submodules
git clone --recurse-submodules https://github.com/ArashMassoudieh/OpenHydroQual.git
cd OpenHydroQual

# 2. Build the library (Linux, CMake)
cd OHQLib
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# 3. Run a Python model
cd ../../PythonBindings
python test_ohq.py
```

See [BUILD.md](BUILD.md) for full platform-specific instructions.

---

## Python API

```python
import openhydroqual_py as ohq

system = ohq.System()
system.set_default_template_path('/path/to/resources/')
system.set_working_folder('/path/to/project/')
system.load_template('main_components.json')
system.add_template('unconfined_groundwater.json')

system.set_system_value('simulation_end_time', '90')
system.set_system_value('initial_time_step',   '0.01')

system.create_block('cell(1:1)', 'Unconfined Groundwater cell', {
    'area': '5000', 'hydraulic_conductivity': '5',
    'piezometric_head': '10.0', 'porosity': '0.3',
})

system.finalize()
system.solve()
system.write_outputs()
```

See [docs/python_api.html](docs/python_api.html) for the full API reference.

---

## License

[MIT License](LICENSE)
