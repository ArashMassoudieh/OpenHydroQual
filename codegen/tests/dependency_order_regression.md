# Cross-object evaluation order regression

The generator previously emitted all per-iteration block quantities before all
link quantities. That assumption fails for block averages over connected links:
urban channel velocity and crop beta_alpha averages read link expressions. Sort
all per-iteration nodes in one dependency graph, retaining a ready-block preference
where dependencies permit it. The same order is used for residual locals and the
member cache consumed by transport/selected outputs. Cross-object cycles reject
with a diagnostic instead of producing broken C++.

Saved executable regression:

```sh
python3 codegen/tests/test_cross_object_order.py GENERATOR RESOURCES NEW_OUTPUT
```

The two synthetic channels and their link are generated, compiled at -O2, and
run, checking water conservation and, with selected outputs enabled, block
velocity equals the connected link's velocity. Before the fix, the locals-only
case failed compilation; the cached-output case compiled but returned stale block
velocities. Both cases pass after the fix. The runner preserves models, generated
headers, binaries, commands and receipts in NEW_OUTPUT and never edits repo inputs.

Independent broader verification is saved under the HydroPINN Dropbox audit
`ohq_remaining_failures_20261119`: all six previously supported example kernels
retain parity; urban now compiles and passes; crop compiles and passes with the
explicit per-block numerical tolerance in its corrected example, including the
full150-day season. The old default-tolerance crop failure is retained: its two
dry native surface storages were approximately -6e-5m³ versus the kernel's zero;
no comparison threshold was loosened. A real state-dependent block/link cycle
also rejects with the expected diagnostic. These are bounded checks, not proof
of support for every expression or transport model.
