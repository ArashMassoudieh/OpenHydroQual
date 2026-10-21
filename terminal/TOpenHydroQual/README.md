# OpenHydroQual Console

`OpenHydroQual-Console` runs an OpenHydroQual model (`.ohq` file) from the command line, without the GUI.
It reads the model, checks it, solves it, writes the output files the model asks for, and reports how
well the model fits any observed data. It is meant for batch runs, scripts, servers and continuous
integration.

The forward model can be solved in two ways:

| | Interpreter (default) | Generated kernel (`--kernel`) |
|---|---|---|
| What solves the model | The OpenHydroQual engine, evaluating the model's expressions at run time | A shared library compiled from the model by the code generator (`.so` on Linux, `.dll` on Windows) |
| Needs | The `.ohq` file, its input files, the resources folder | The same, plus the library generated from that model |
| Speed | Baseline | Faster; the gain depends on the model (a one-year pond model with three observations: 2.6 s vs 0.18 s) |
| Writes the bulk output file (`alloutputfile`) | Yes | No |
| Writes the observed output file (`observed_outputfile`) | Yes | Yes |
| Prints fit statistics for observations with data | Yes | Yes |
| Model changes take effect | Immediately | Only after regenerating the kernel, except calibration parameter values |

The two paths give the same observations to rounding; the kernel replaces only the solve.

---

## 1. Building

### Linux

Dependencies: Qt 6 (Core), Armadillo, LAPACK, BLAS, GSL, SuperLU, OpenMP.

```bash
cd terminal/TOpenHydroQual
qmake6 -o Makefile TOpenHydroQual.pro
nice -n 19 make -j4
```

This produces `terminal/TOpenHydroQual/OpenHydroQual-Console`.

### Windows

Either of these:

- **Visual Studio:** open `terminal\TOpenHydroQual.sln`, select the **OHQTerminal** project, configuration
  **Release | x64**, and build. The executable is `terminal\x64\OHQTerminal.exe`. The LAPACK/BLAS DLLs are
  copied next to it by the post-build step.
- **Qt Creator or qmake with MSVC:** open `TOpenHydroQual.pro` (or run `qmake TOpenHydroQual.pro` then
  `nmake` / `jom` from a Visual Studio x64 command prompt). The executable is `OpenHydroQual-Console.exe`.

In the commands below, use whichever of the two names your build produced.

---

## 2. Running with the interpreter

### Syntax

```
OpenHydroQual-Console <model.ohq> [options]
```

### Options

| Option | Argument | Meaning |
|---|---|---|
| `-o`, `--outputfile` | file name | Write the bulk outputs to this file, overriding the model's `alloutputfile` setting. Relative to the working folder. |
| `-w`, `--workdir` | folder | Working folder: where relative input paths are resolved and outputs are written. Default: the folder that contains the `.ohq` file. |
| `-r`, `--resources` | folder | Folder with the plugin templates (`main_components.json`, the other `*.json` plugins and `settings.json`). See below for the default. |
| `-q`, `--quiet` | | Do not print per-step solver progress. |
| `--skip-verify` | | Do not check property values against the limits declared in the templates before solving. Not recommended. |
| `-k`, `--kernel` | library path | Solve with a generated kernel (section 3). |
| `-h`, `--help` | | Print the usage summary and exit. |

The model file can appear before or after the options. An unknown option prints the usage summary and
exits with code 2.

### Resources folder

The templates named in the model (`loadtemplate` / `addtemplate` lines) are looked up in the resources
folder, chosen in this order:

1. `--resources <folder>`
2. the environment variable `OHQ_RESOURCES`
3. `<folder of the executable>/../../resources/` (works inside the source tree)

Template paths in the `.ohq` file can be bare file names (`addtemplate; filename = Pond_Plugin.json`);
absolute paths saved by older versions of the GUI are reduced to their file name.

### Examples

Linux:

```bash
export OHQ_RESOURCES=/path/to/OpenHydroQual/resources
./OpenHydroQual-Console /data/models/pond.ohq -q
./OpenHydroQual-Console pond.ohq -o pond_results.txt -w /tmp/run1 -r /path/to/OpenHydroQual/resources
```

Windows, Command Prompt:

```bat
set OHQ_RESOURCES=C:\OpenHydroQual\resources
OpenHydroQual-Console.exe C:\models\pond.ohq -q
OpenHydroQual-Console.exe pond.ohq -o pond_results.txt -w C:\runs\run1
```

Windows, PowerShell:

```powershell
$env:OHQ_RESOURCES = "C:\OpenHydroQual\resources"
.\OpenHydroQual-Console.exe C:\models\pond.ohq -q
```

### What happens during a run

1. **Read and build the model.** Every line of the script is executed. If any line fails (an unknown
   type or property, a missing input file), the console lists the errors and stops with exit code 4. It
   does not solve a model that is not the one you wrote.
2. **Check property limits.** Every property is checked against the criteria declared in its template
   (for example a channel length must be positive). Violations stop the run with exit code 4, unless
   `--skip-verify` is given.
3. **Apply solver settings** from the model (`initial_time_step`, tolerances, `write_interval`, and so on).
   If `write_solution_details` is on, a diagnostic log is written to `solution_details.txt` in the
   working folder.
4. **Apply calibrated parameters.** If the model defines `Parameter` objects, their current values are
   pushed into the properties they control before solving, so a calibrated model runs with its fitted
   values.
5. **Solve.** Messages logged during the solve (for example a property that a block type does not
   define, which evaluates to 0) are printed as `Warning:` lines. The run continues and they do not
   change the exit code. A solve that fails exits with code 5.
6. **Write observations.** If the model sets `observed_outputfile`, every observation is written there,
   resampled onto the `initial_time_step` grid.
7. **Report fit statistics.** For every observation that has observed data, the console prints MSE, R²,
   NSE and the negative log-likelihood. EMC observations also get a per-event table in
   `<observation name>_EMC.txt`.
8. **Write bulk outputs.** Every quantity marked for output is written to `alloutputfile` (or the
   `--outputfile` name), resampled onto the `initial_time_step` grid. Nothing is written if the file name
   is empty or `record_results` is `No`.

Output timing: each value is stamped at the end of the solver step that produced it. When resampling,
states (storages, depths, concentrations, heads) are interpolated linearly; rates (link flows, mass
transfer, sources, input series) are averaged over each grid interval, so integrating a resampled rate
reproduces the volume or mass the solver moved.

### Output file format

Comma-separated, one header line, then one row per output time. Each series occupies two columns,
time then value (values below are illustrative):

```
t, Pond_Storage,t, Pond_depth,t, Weir_flow
0.00000,10101.9,0.00000,1.5,0.00000,0
0.01000,10098.2,0.01000,1.4996,0.01000,38.11
```

Series names are `<object>_<quantity>` (for constituents `<block>_<constituent>:<quantity>`), and
observations appear under their own names in the observed output file.

---

## 3. Running with a generated kernel

The code generator turns one specific model into C++ in which every equation is written out, then
compiles it into a shared library. The console loads that library and uses it in place of the
interpreter for the solve. The same mechanism is used by `OHQ-LM`, `OHQ-GA` and `OHQ-MCMC`
(`terminal/OHQ-Common/ohq_kernel.h`).

### Step 1. Generate the kernel project

**From the GUI (Windows and Linux):** open the model and choose **Model › Export to C++…**. Pick a
class name and an output folder, set **Target** to **Shared library**, and tick **Build generated
project (Release)** if CMake was found. The GUI writes a self-contained project folder (generated
header, header-only runtime, `CMakeLists.txt`, C-API wrapper).

**From the command line (Linux):**

```bash
cd /folder/with/the/model          # the generator resolves input files from here
/path/to/OpenHydroQual/codegen/build/ohq_generate \
    pond.ohq /path/to/OpenHydroQual/resources ./pond_kernel Pond --project shared
```

Arguments: `<model.ohq> <resources_dir> <out_dir> <ClassName> [stateVar] [--project exe|lib|shared]
[--outputs <file>]`. Use `--project shared` for the console. The class name becomes the library name
(`libPond.so`, `Pond.dll`). `stateVar` defaults to `Storage`.

### Step 2. Build the library

Linux:

```bash
cmake -S pond_kernel -B pond_kernel/build -DCMAKE_BUILD_TYPE=Release
cmake --build pond_kernel/build -j4
# result: pond_kernel/build/libPond.so
```

Windows (Developer Command Prompt or PowerShell, with CMake and Visual Studio 2022):

```bat
cmake -S pond_kernel -B pond_kernel\build -G "Visual Studio 17 2022" -A x64
cmake --build pond_kernel\build --config Release
rem result: pond_kernel\build\Release\Pond.dll
```

The kernel must be built for the same architecture as the console (x64).

### Step 3. Run

Linux:

```bash
./OpenHydroQual-Console pond.ohq -q --kernel pond_kernel/build/libPond.so
```

Windows, Command Prompt:

```bat
OpenHydroQual-Console.exe pond.ohq -q --kernel pond_kernel\build\Release\Pond.dll
```

Windows, PowerShell:

```powershell
.\OpenHydroQual-Console.exe pond.ohq -q --kernel .\pond_kernel\build\Release\Pond.dll
```

The `.ohq` file is still required: it supplies the observations, their observed data, the calibration
parameters and the output file names. Only the solve is done by the kernel.

### What the console checks before using a kernel

```
Forward model: generated kernel pond_kernel/build/libPond.so
Kernel class    : Pond
Kernel verified : 3 parameters, 3 observations, names match the model; all 3 kernel-owned parameters move it.
```

- The library must load and export the kernel interface.
- The kernel's parameters and observations must match the model's, by count and by name.
- Each parameter the kernel owns is perturbed once to confirm that it actually changes the result.

If any check fails, the console prints the reason and exits with code 6, for example:

```
Model parameter 'p_k' is not exposed by the kernel. Regenerate the kernel from this model.
*** the kernel cannot be used with this model.
```

### What a kernel run produces

- `observed_outputfile`: every observation, resampled onto the `initial_time_step` grid, same format as
  an interpreter run.
- Fit statistics for observations with observed data, and EMC tables, as in section 2.
- **No bulk output file.** The kernel returns observations and objective functions only. The console
  says so at the end of the run. To see other quantities, declare them as observations before
  generating the kernel, or run the interpreter.

### When to regenerate the kernel

The kernel is compiled from the model as it was when it was generated. Input time series are embedded
in it, and so are the start time, end time and time step settings. Regenerate and rebuild after any change
to:

- blocks, links, sources, constituents, reactions or their properties;
- input files (inflows, rainfall, weather, loads);
- simulation or solver settings;
- the list of observations or parameters.

Changes that do **not** need a new kernel:

- the values of `Parameter` objects (pushed into the kernel at every run);
- observed data files and error-model settings of observations (handled by the console).

### Other notes

- EMC observations need a matching flow observation on the same object to be evaluated with the kernel;
  without one, the console falls back to the interpreter for that run.
- `maximum_time_allowed` in the model's settings also limits a kernel run; a run that exceeds it is
  reported as a failed solve.

---

## 4. Exit codes

| Code | Meaning |
|---|---|
| 0 | Success. Warnings logged during the solve do not change this. |
| 2 | Usage error: no model file, an unknown option, or `--help`. |
| 3 | The model file or the resources folder was not found. |
| 4 | The model could not be built as written, or a property violates its declared limits. Nothing was solved. |
| 5 | The solve failed. |
| 6 | `--kernel`: the library was not found, could not be loaded, or was not generated from this model. |

Always check the exit code in scripts.

Bash:

```bash
for f in scenarios/*.ohq; do
    ./OpenHydroQual-Console "$f" -q || echo "run failed: $f (exit $?)"
done
```

PowerShell:

```powershell
Get-ChildItem scenarios\*.ohq | ForEach-Object {
    & .\OpenHydroQual-Console.exe $_.FullName -q
    if ($LASTEXITCODE -ne 0) { Write-Host "run failed: $($_.Name) (exit $LASTEXITCODE)" }
}
```

---

## 5. Troubleshooting

| Message | Cause and fix |
|---|---|
| `Resources folder not found` | Pass `--resources` or set `OHQ_RESOURCES` to the folder that contains `main_components.json`. |
| `error(s) while building the model` | A line in the `.ohq` failed: check the listed property names against the plugin templates, and that every input file exists relative to the working folder. |
| `property value(s) violate the criteria` | Fix the listed values; `--skip-verify` runs anyway but the results may not be meaningful. |
| `Warning: property 'inflow' does not exist in '<block>'` | A constituent term is evaluated on a block type that has no inflow (for example a `fixed_head` boundary). It evaluates to 0 and does not affect the results. |
| `Kernel library not found` | Check the path given to `--kernel`. On Windows the library is under `build\Release\`. |
| `... is not exposed by the kernel` / counts differ | The kernel was generated from a different version of the model. Regenerate it. |
| Library fails to load on Windows | The console and the kernel must both be x64 and built with compatible Visual Studio runtimes; also check that the path has no typo. |
| Bulk output file missing after a kernel run | Expected: use the observed output file, or run without `--kernel`. |
