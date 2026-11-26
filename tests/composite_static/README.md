# Static checks for composite templates and .ohq scripts

These scripts never build or run a model. They catch the load-time mistakes
that otherwise only show up as errors when the GUI or console opens a model.

- `check_composites.py [-v] [template.json ...]`: checks every composite in
  `resources/` (or the files given). It verifies that member and link types
  exist, that `applyto` targets name real member/link properties, and that
  non-numeric targets (expressions, sources, time series) are mapped from a
  bare composite property, as `Composite::Propagate()` requires. `-v` also
  lists warnings, such as types that come from a template outside the
  file's `requires` chain.
- `check_ohq.py model.ohq [...]`: checks that every `create` line in a script
  uses a known type and known properties, that links join objects defined
  above, and that a link attached to a composite has a port for that link
  type on that end (`external_links`).

```
python3 tests/composite_static/check_composites.py
python3 tests/composite_static/check_ohq.py Examples/Storm_Sewershed/storm_sewershed_example.ohq
```
