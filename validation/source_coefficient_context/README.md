# Receiver-context source coefficient initialization

Source coefficients are evaluated by `Source::GetValue(receiver)` in the
receiving object's context. For example, a shared Precipitation source has
`coefficient = area`; each receiving catchment supplies its own area. Rain
itself has no area. `System::CalculateAllExpressions` already excludes source
coefficients from generic source-context evaluation. `Object::CalculateInitialValues`
must make the same exclusion when preparing past values of derived expressions.

The fix skips only the source object's `coefficient` expression in that generic
initialization pass. It neither adds a fictitious source area nor changes source
evaluation, templates, physical parameters or validation criteria.

Executable regression (from a sibling GIStoOHQ checkout):

```
python3 scripts/verify_ohq_rainfall_ledger.py CONSOLE RESOURCES NEW_OUTPUT
```

The script uses the real GIStoOHQ rainfall writer, a known native rainfall file,
two catchments of1000/2500m² sharing Rain and a collector with fixed discharge.
It checks receiver-specific rain rates, an independent analytical storage
solution and rainfall minus storage/discharge/losses. It repeats with60%runoff
and first-order loss, then swapped receiver areas. Numerical guards reject
nonfinite output. It does not use observations or fit parameters.

19November2026 evidence: all3cases passed before/after the change, output tables
byte-identical; the erroneous Rain-area warning disappeared. Conservative mass
residual was1.14e-15m³; the loss case was-4.21e-5m³ on3.5m³ rainfall (finite
output precision and integration tolerance). All4generated-model short startup
outputs also stayed byte-identical; invalid length/moisture controls still
reject. Exact binaries/source hashes and archived evidence remain under
HydroPINN paper Dropbox/audits/ohq_rainfall_ledger_20261119. This is synthetic
source/balance validation, not full-HRU/field-event/scientific skill acceptance.
