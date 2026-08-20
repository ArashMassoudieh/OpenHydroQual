# Outstanding Issues

Working notes from the session of 2026-08-19, on branch `claude/serene-hodgkin-d976ec`.
Ordered roughly by severity. Each entry records what was actually observed, so we
can pick any of them up without re-deriving the diagnosis.

Confidence is marked per item:
- **Confirmed** — reproduced by running code, evidence quoted.
- **Read-only** — found by reading the source, not executed.

---

## Already fixed on this branch

Recorded here only so we know what is done and what these notes assume.

1. **`System::ReadSystemSettingsTemplate` printed a spurious parse failure on every load.**
   `file >> root` consumed the whole stream, then `reader.parse(file, root, true)`
   re-parsed the same now-exhausted stream and always failed at line 1 column 1.
   Replaced with a single `Json::parseFromStream` + `CharReaderBuilder`, matching
   `MetaModel::AppendFromJsonFile`, returning `false` with the real jsoncpp error.
   `aquifolium/src/System.cpp:3408`. Also applied to the main checkout on 2026-08-19.

2. **The `name` quantity was blanked whenever quantities were reset from the metamodel.**
   All three `Object::SetQuantities` overloads that take a `MetaModel`/`System`
   replace `var` wholesale and call `SetDefaults()`, reverting the `name` quantity
   to the template default. Callers had to write the name back afterwards, by name,
   one call site at a time. Added `Object::SyncNameQuantity()`, called from all
   three overloads. `aquifolium/src/Object.cpp:334`, `aquifolium/include/Object.h:71`.
   Also applied to the main checkout on 2026-08-19.

---

## 1. Blank link names in the property box — **RESOLVED 2026-08-19**

**Status:** closed. Both halves are now in the main checkout's uncommitted work and
the combination has been verified. Kept here for the record; nothing to revisit.

`MainWindow::AddLink` in the main checkout now binds the edge to the link's *actual*
name (`link.GetName()`) rather than the requested one, with a null guard — which
correctly fixes the wrong-object binding. As part of that it also dropped the old
write-back line:

```cpp
system.object(LinkName.toStdString())->SetName(LinkName.toStdString());
```

That line was the only thing repopulating the `name` quantity after
`System::AddLink` reset it from the metamodel. Without it, **every** newly drawn
link shows a blank name in the property window, not just parallel ones — which
matches the reported symptom exactly.

**What is needed:** `Object::SyncNameQuantity()` and its three call sites
(`aquifolium/src/Object.cpp`, `aquifolium/include/Object.h` on branch
`claude/serene-hodgkin-d976ec`) must land alongside the `mainwindow.cpp` change.
With both, a renamed parallel link carries its name correctly:

```
link.GetName() after AddLink = 'Reactor_1 - Reactor_2(0:0) (2)'
NEW LINK name-quan='Reactor_1 - Reactor_2(0:0) (2)'
```

Without `SyncNameQuantity()`, the same run gives `name-quan=''`.

**Confirmed in the GUI, 2026-08-19:** after the `mainwindow.cpp` fix landed in the
main checkout, link names are *still* blank in the property box when the app is built
there. That is this issue, not a separate one — the main checkout has the GUI half
and not the engine half. To see names populate, `aquifolium/src/Object.cpp` and
`aquifolium/include/Object.h` from branch `claude/serene-hodgkin-d976ec` have to be
present in the tree being built:

```
cp .claude/worktrees/serene-hodgkin-d976ec/aquifolium/src/Object.cpp aquifolium/src/Object.cpp
cp .claude/worktrees/serene-hodgkin-d976ec/aquifolium/include/Object.h aquifolium/include/Object.h
```

Blocks are unaffected because their write-backs at `mainwindow.cpp:1039` etc. are
still in place — which is why only edges look broken, parallel or not.

**Attribution, checked against the main checkout on 2026-08-19** (recorded because
this looked at first like a regression from the engine-side work):

| Check | Result |
| --- | --- |
| `grep -c SyncNameQuantity aquifolium/src/Object.cpp` in main checkout | `0` |
| `grep -c SyncNameQuantity aquifolium/include/Object.h` in main checkout | `0` |
| `git status` in main checkout | `Object.cpp` / `Object.h` not modified |
| Link name write-back in `mainwindow.cpp` | removed by the `AddLink` edit |

So the engine-side change is *not* in the binary showing blank edge names — it only
exists on branch `claude/serene-hodgkin-d976ec`. The blank name comes from the
`system.object(LinkName)->SetName(LinkName)` line that the `AddLink` fix removed;
nothing else repopulates the `name` quantity after `System::AddLink` resets
quantities from the metamodel.

Two ways to close it, whichever fits better when we pick this up:

1. Take `Object.cpp` / `Object.h` from this branch, so the name is maintained
   centrally for every object type and no call site has to remember (this is what
   "name should always be added automatically" asks for). Then delete the seven
   remaining write-backs listed in issue #2.
2. Or re-add a single write-back in `AddLink` using the corrected name —
   `newlink->SetName(actualname.toStdString());` — which fixes edges only and leaves
   the underlying blanking in place for every other caller.

**Resolution:** option 1. `Object.cpp` / `Object.h` were copied from this branch into
the main checkout (both were pristine at HEAD there, so nothing was overwritten).
Verified two ways:

- `mainwindow.cpp` from the main checkout compiles against the new `Object.h`
  (`g++ -fsyntax-only`, exit 0).
- A harness replicating the *current* `MainWindow::AddLink` sequence — binding by
  `link.GetName()` with no `SetName()` write-back, i.e. relying entirely on the
  central fix — linked against the main checkout's `System.cpp`:

```
pass 1: property box would show Name = 'Reactor_1 - Reactor_2(0:0)'   (link is 'Reactor_1 - Reactor_2(0:0)')
A link named 'Reactor_1 - Reactor_2(0:0)' already exists; the new link was named 'Reactor_1 - Reactor_2(0:0) (2)'
pass 2: property box would show Name = 'Reactor_1 - Reactor_2(0:0) (2)'   (link is 'Reactor_1 - Reactor_2(0:0) (2)')
```

**Confirmed fixed in the GUI by the author after rebuilding.** Link names now show
in the property box, for single and parallel links alike.

Issue #2 (deleting the seven now-redundant write-backs) remains open as follow-up.

---

## 2. Redundant name write-backs elsewhere in the GUI — **RESOLVED 2026-08-19**

**Where:** seven remaining occurrences in `mainwindow.cpp` — lines 1039, 1069, 1231,
1317, 1348, 1380, 1414 — plus several already commented out nearby.

```cpp
system.object(name)->SetName(name);
```

Each exists only to undo the blanking that `Object::SyncNameQuantity()` now prevents
centrally. They are harmless but misleading: they imply the name needs manual
restoration, which is what led to the link bug when one of them was removed. Worth
deleting in one pass once #1 has landed and been tested.

Note they also dereference `system.object(name)` without a null check, the same
pattern that was just guarded in `AddLink`. That was a live crash path: for
constituents and reaction parameters, `Object::SetName` refuses names containing
parentheses (issue #10), so the preceding `X.SetName(name)` could fail, leaving
`system.object(name)` returning `nullptr` and this line dereferencing it.

**Resolution:** all seven removed from `mainwindow.cpp`. Each `System::Add*` calls
`SetQuantities(metamodel, type)` on the stored object, which is exactly where
`Object::SyncNameQuantity()` now restores the name — verified for all six
`Add*` variants by reading them, and functionally for `AddBlock` and `AddLink`:

```
BLOCK (no write-back): property box Name = 'New Reactor 1'   (object is 'New Reactor 1')
LINK  (first):         property box Name = 'Reactor_1 - Reactor_2(0:0)'
LINK  (parallel):      property box Name = 'Reactor_1 - Reactor_2(0:0) (2)'
```

`AddComposite` was doubly redundant — it already calls `added->SetName(cmp.GetName())`
itself. The commented-out copies of this line nearby were left alone.

---

## 3. Diagnostics on `std::cout` are lost when the app does not exit cleanly — **Confirmed**

**Where:** `System.cpp` and friends generally; anything using `std::cout` for
user-facing diagnostics.

`std::cout` is block-buffered when redirected to a file, so messages written shortly
before a kill never reach the file. This actively misled us while verifying the
settings-parse fix: the first "after" run looked clean for the wrong reason, and the
"before" run showed no message either. Re-running under `stdbuf -o0` made both
reproduce reliably.

**Impact:** anyone diagnosing from a redirected log can conclude a message is absent
when it was merely dropped. Worth routing engine diagnostics through the existing
log window / `ShowMessage` path, or at minimum flushing them.

---

## 4. Console targets compare `char*` to `""` — **Read-only**

**Where:** `terminal/TOpenHydroQual/main.cpp:36`, `terminal/OHQConsole/main.cpp:31`.

```cpp
if (argv[1]=="")
```

Pointer comparison, never true, so the intended empty-filename guard never fires.
The `argc<2` check above it covers the missing-argument case, so this is dead code
rather than a live crash — but it should be `std::string(argv[1]).empty()` or deleted.

---

## 5. `TOpenHydroQual.pro` has wrong relative paths and cannot build — **Read-only**

**Where:** `terminal/TOpenHydroQual/TOpenHydroQual.pro:13`, `109`–`111`, `148`–`160`.

Refers to `../../../jsoncpp/...`, which from `terminal/TOpenHydroQual` resolves to the
*parent of the repository*. Should be `../../jsoncpp/...`. This is why the console
build was not usable for verification in this session and the GUI target had to be
used instead. `terminal/TOpenHydroQual/build/Desktop-Debug/` is empty, suggesting it
has not built successfully in some time.

---

## 6. Example models embed absolute, machine-specific paths — **Confirmed**

**Where:** all 13 `Examples/*.ohq`.

```
loadtemplate; filename = /home/arash/Projects/OpenHydroQual/build/Desktop_Qt_6_8_3-Debug/../../resources/main_components.json
```

Every example hard-codes `/home/arash/...` and a build directory that no longer exists
(`Desktop_Qt_6_8_3-Debug`). It only resolves today because the `../../` in the middle
happens to climb back to the repository root. These will not load on any other machine,
which makes the shipped examples unusable for new users.

**Fix:** write `loadtemplate` paths relative to the resources directory, and have the
loader resolve them against `DefaultTemplatePath()`.

---

## 7. `qplotwindow.h` listed twice in `OpenHydroQual.pro` — **Confirmed**

**Where:** `OpenHydroQual.pro:201` and `OpenHydroQual.pro:262` (both in `HEADERS`).

Every `make` run emits:

```
Makefile:10648: warning: overriding recipe for target 'moc_qplotwindow.cpp'
Makefile:4547: warning: ignoring old recipe for target 'moc_qplotwindow.cpp'
```

Harmless today, but it means the moc rule is generated twice. Delete one entry.

---

## 8. Stale `HEADERS` entries for jsoncpp — **Confirmed**

**Where:** `OpenHydroQual.pro:245`, `247`, `255`.

qmake warns on every configure:

```
WARNING: Failure to find: jsoncpp/include/json/autolink.h
WARNING: Failure to find: jsoncpp/include/json/features.h
WARNING: Failure to find: jsoncpp/src/lib_json/version.h.in
```

The vendored jsoncpp no longer ships `autolink.h`, and `features.h` was renamed to
`json_features.h`. Update or drop these three lines.

---

## 9. Dead signal/slot connections at startup — **Confirmed**

Every launch logs six broken connections:

```
QMetaObject::connectSlotsByName: No matching signal for on_check_object_browser()
QMetaObject::connectSlotsByName: No matching signal for on_check_showlogwindow()
QMetaObject::connectSlotsByName: No matching signal for on_object_browser_closed(bool)
QMetaObject::connectSlotsByName: No matching signal for on_actionRecent_triggered()
QObject::connect: No such slot DiagramView::sceneChanged() in diagramview.cpp:42
QObject::connect: No such signal MainWindow::closed() in mainwindow.cpp:190
```

The first four are auto-connect slots whose signals no longer exist (renamed or
removed widgets). The last two are explicit `connect()` calls that silently do
nothing — worth checking whether `diagramview.cpp:42` and `mainwindow.cpp:190` were
meant to wire up behaviour that is currently missing, rather than just deleting them.

---

## 10. `Object::SetName` fails silently for constituents and reaction parameters — **Read-only**

**Where:** `aquifolium/src/Object.cpp:402`.

Returns `false` without any error message or log entry when a constituent's or
reaction parameter's name contains `(` or `)`. Callers throughout `System` and the GUI
ignore the return value, so a rejected rename looks like it succeeded. Should at least
append to the error handler.

---

## 11. `jsoncpp` submodule is not initialized in git worktrees — **Confirmed**

Creating a worktree leaves `jsoncpp/` empty, so nothing builds until it is populated
by hand. Worth a note in the README or a bootstrap script
(`git submodule update --init --recursive`, or `--reference` against the main clone).

Also worth knowing: copying the directory from the main checkout brings its `.git`
file along, which breaks `git status` in the worktree with
`fatal: not a git repository: jsoncpp/../.git/modules/jsoncpp`. Delete that file
after copying.

---

## Build notes for whoever picks these up

- Shadow build used this session: `build/Debug`, configured with
  `qmake ../../OpenHydroQual.pro -spec linux-g++ CONFIG+=debug -after QMAKE_CXXFLAGS+=-O0`.
  The `-after` is what makes `-O0` win over the `.pro`'s `-O3 -march=native`; a full
  build then takes a few minutes instead of far longer.
- Build with `make -j2` and redirect output to a file.
- To exercise a model load headlessly, `QT_QPA_PLATFORM=offscreen` works, and
  `stdbuf -o0` is required to see `std::cout` diagnostics (see issue #3).
