# Headless parameter estimation for OpenHydroQual

Two console runners that do what the GUI's **Optimize** and **Inverse Run**
buttons do, without Qt Widgets, so calibration can run on a cluster or in a
loop:

| | |
|---|---|
| `terminal/OHQ-GA/` | genetic-algorithm calibration (`MainWindow::onoptimize`, headless) |
| `terminal/OHQ-MCMC/` | Bayesian estimation + realizations (`MainWindow::oninverserun`, headless) |
| `terminal/OHQ-Common/` | headers shared by both — `ohq_common.h`, `ohq_kernel.h` — and this README |

Both take an `.ohq` model and read everything they need out of it: the
`parameter` entries define what is estimated and their priors, the `observation`
entries define the likelihood. Nothing about the runners is specific to any
model.

Each runner can evaluate the forward model two ways:

* **interpreter** (default) — `System::Solve()`, the same code path as the GUI.
* **codegen kernel** (`--kernel`) — a shared library compiled from the model by
  `ohq_generate`. Typically ~10-15x faster; see
  [Running against a codegen kernel](#running-against-a-codegen-kernel).

Only the *solve* is replaced. Parameter bookkeeping, the objective function, the
GA/MCMC drivers and all output writing are identical either way, so the two
modes are directly comparable.

---

## 1. Building the runners

Dependencies (Ubuntu):

```bash
sudo apt-get install libarmadillo-dev liblapack-dev libblas-dev libgsl-dev \
                     libopenblas-dev libsuperlu-dev qt6-base-dev
```

```bash
cd terminal/OHQ-GA          # or terminal/OHQ-MCMC
mkdir -p build && cd build
qmake6 ../OHQ-GA.pro
make -j$(nproc)
```

`OHQ` defaults to `$$PWD/../..`, i.e. this checkout, and `JSONCPP` to
`$$OHQ/jsoncpp`, the vendored copy — so an in-tree build needs no arguments.
qmake stops with an error if the aquifolium sources are not where it expects.
To build against a different checkout:

```bash
qmake6 ../OHQ-GA.pro OHQ=/path/to/OpenHydroQual
```

These projects compile ~38 aquifolium `.cpp` files directly from
`$$OHQ/aquifolium/src` rather than linking `libOHQLib`. That is deliberate: they
cannot silently run against a stale library. It also means **a change to an
aquifolium header requires rebuilding the runner**, not just relinking.

## 2. Running with the interpreter

```bash
./OHQ-GA   model.ohq [working_folder]
./OHQ-MCMC model.ohq [working_folder]
```

`working_folder` defaults to the directory containing `model.ohq`, and is
created if missing. GA writes `GA_output.txt`, `outputs.txt`,
`observedoutputs.txt` and `errors.txt` there. MCMC writes the chain to the
filename given by the model's `outputfilename` setting.

### Options (both runners)

| flag | effect |
|---|---|
| `--kernel <lib>` | evaluate the forward model with a codegen shared library instead of the interpreter |
| `--continue`, `-c` | GA: seed the initial population from an existing `GA_output.txt` in the working folder. MCMC: append to the existing chain |
| `--sparse` | use the sparse linear solver |
| `--dependency-jacobian` | build the Jacobian from the dependency graph rather than dense finite differences |
| `--verify-jacobian` | cross-check the analytical/dependency Jacobian against finite differences and report |

---

## Running against a codegen kernel

This is a three-step pipeline: **generate C++ → build a shared library → point
the runner at it.** The kernel is compiled from one specific `.ohq` file and is
only valid for that file; see [Keeping the kernel in
sync](#keeping-the-kernel-in-sync).

### Step 1 — generate the C++ project

```bash
ohq_generate <model.ohq> <resources_dir> <out_dir> <ClassName> [stateVar] \
             --project exe|lib|shared
```

`ohq_generate` is built from `codegen/`:

```bash
cmake -S codegen -B codegen/build -DCMAKE_BUILD_TYPE=Release
cmake --build codegen/build -j$(nproc)
```

For calibration you want **`--project shared`**, which emits the fixed-name
`ohq_kernel_*` C ABI that `--kernel` loads with `dlopen`. (`exe` emits a
standalone solver that writes a CSV — useful for a single forward run or a
parameter sweep, but `--kernel` cannot load it. `lib` emits a static library
with class-prefixed symbols for linking into your own C++.)

```bash
codegen/build/ohq_generate model.ohq \
    /path/to/OpenHydroQual/resources \
    gen_model S13 Storage --project shared
```

* `<resources_dir>` is the repo's `resources/` folder — the templates the model
  file refers to.
* `<ClassName>` names the generated class, the library and its symbols:
  `S13` produces `libS13.so`. Any valid C++ identifier.
* `[stateVar]` is the state quantity to integrate, usually `Storage`.

The generator **refuses to emit a kernel a calibration cannot drive**: if any
`parameter` in the model does not reach a settable value in the generated code,
generation fails with

> `CodeGenerator: estimated parameter '<name>' ... Refusing to emit a kernel that would silently ignore it.`

Do not work around this. It means the calibration would have varied that
parameter while the model ignored it, and the GA/MCMC would have reported
confident results for a parameter that did nothing.

### Step 2 — build the shared library

The generated `out_dir` is a self-contained CMake project — it carries its own
copy of the header-only runtime in `runtime/` and needs nothing from this repo
at compile time:

```bash
cmake -S gen_model -B gen_model/build -DCMAKE_BUILD_TYPE=Release
cmake --build gen_model/build -j$(nproc)
```

This produces `gen_model/build/libS13.so`, plus a small `S13_example`
executable that verifies the library links and runs.

Use `-DCMAKE_BUILD_TYPE=Release`. A Debug kernel is roughly an order of
magnitude slower and gives back everything the kernel was for.

### Step 3 — run the calibration

```bash
./OHQ-GA   model.ohq work_dir --kernel /abs/path/to/gen_model/build/libS13.so
./OHQ-MCMC model.ohq work_dir --kernel /abs/path/to/gen_model/build/libS13.so
```

Pass the path to the `.so` itself, not its directory. An absolute path is
safest; a relative one is resolved by `dlopen` against the current directory,
not the working folder.

### What you should see at startup

The runner resolves the 19 `ohq_kernel_*` symbols it needs (a `--project shared`
kernel exports 40), then checks the library against
the model it was handed:

```
Kernel class    : S13
Kernel verified : 8 parameters, 16 observations, names match the model.
```

If instead you see a parameter/observation **count** mismatch or a **name**
mismatch at index *i*, the kernel was generated from a different version of the
model. Regenerate it — do not proceed.

A **missing-symbol** line is not fatal and the run will continue, so read the
startup output rather than assuming silence means success. In particular a
kernel predating `ohq_kernel_step_to` still loads, but the runner then falls
back to `run_to`, which has no wall-clock limit — an MCMC proposal that hits a
stiff region will hang instead of being rejected on the clock. Regenerate.

### Keeping the kernel in sync

The verification above compares parameter and observation **names and counts**.
That catches the common mistakes, but it is not a full equivalence check: edits
that change the *physics* while leaving those names intact — a different
reaction expression, a changed initial condition, a new solver setting — will
**not** be caught, and the calibration will silently optimize the old model.

The safe rule: **regenerate the kernel whenever the `.ohq` changes.** It takes
seconds and removes the whole class of error. Wiring steps 1 and 2 into the
script that launches the calibration is the usual way to guarantee it.

Two more sync rules for anyone editing this repo:

* Edited anything under `codegen/runtime/`? Re-run
  `codegen/tools/embed_runtime.py` before rebuilding `ohq_generate` — the
  runtime is embedded in the generator binary, and a stale embed means newly
  generated projects silently carry the old runtime.
* Edited an `aquifolium/include/` header? Rebuild `OHQLib` **and** the runners.
  A prebuilt `libOHQLib.so` against a changed `System.h` is an ABI mismatch that
  shows up as a segfault in `ohq_generate`, far from its cause.
* `ohq_kernel.h` is the consumer half of the ABI emitted by
  `codegen/src/CodeGenerator.cpp`. Change one, change the other, and bump
  `ohq_kernel_abi_version`.

### Interpreter vs kernel: expected agreement

The kernel and the interpreter are held to bit-level parity on the same model at
the same solver settings, and the runtime ports the interpreter's adaptive
stepping, `optimize_lambda`, damping policy, chord-Jacobian refresh and
oscillation control. On the two-site sorption models used to develop this,
141-step runs agreed on 140 of 141 steps with a median breakthrough-curve
difference of -0.010%.

Residual disagreement is not expected. If the kernel and the interpreter give
materially different objective values on the same model, treat it as a bug and
check `issues.md` before trusting either.

## Performance

Measured on a 16-column two-site sorption model, 1000 steps:

| | wall time |
|---|---|
| interpreter | 278 s |
| codegen kernel | ~20 s |

Roughly 14x. The gain is larger for models with many reaction terms, and it is
multiplied by the population size or chain length — which is what makes the
codegen path worth the extra two steps for a real calibration.
