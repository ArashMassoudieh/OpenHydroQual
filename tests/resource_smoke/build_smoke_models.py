#!/usr/bin/env python3
"""Minimal regression smoke test for the riparian_urban_hru.json /
unconfined_groundwater.json additions: hillslope_width_scale/
hillslope_length_scale (on the default Riparian_Urban_HRU), and the two
additive experimental composites Riparian_Urban_HRU_OneWayRecharge and
Riparian_Urban_HRU_SymmetricKRecharge (each paired with its own additive
connector type).

No automated test previously covered any of this (flagged in review). This
is deliberately NOT a C++ codegen/parity test like the ones in
codegen/tests/ -- those test the codegen pipeline itself against its own
fixture models, a different concern from "does this resource template
actually build and solve."

PORTABILITY, STATED PRECISELY (a prior version of this docstring blurred
this distinction, flagged in review): the generated/*.ohq files THIS
SCRIPT PRODUCES, once committed, are portable and dependency-free -- every
path inside them is relative to the .ohq's own directory, and
run_smoke_test.sh needs nothing but a console binary and this checkout to
run them. THIS SCRIPT ITSELF IS NOT PORTABLE: SOURCE_OHQ below is an
absolute, machine-specific path into a different project's Dropbox-synced
working directory that most checkouts of this repository will not have.
build_smoke_models.py is authoring-only tooling for regenerating the
fixtures when the underlying topology needs to change -- it is not run as
part of the smoke test and does not need to work on every machine. If you
need to regenerate these fixtures somewhere SOURCE_OHQ doesn't exist,
either point SOURCE_OHQ at a local copy of that CLI-build output, or treat
the committed generated/*.ohq files as the source of truth and hand-edit
them (they are small, plain-text .ohq scripts).

Derived from OHQ_Model/v2_development/structural_experiments.py's proven
upstream-extraction pattern (same CLI-build source line format), reduced to
a single subbasin (Subbasin_9 -> Reach_19 -> Reach_20 -> Outlet) and a
10-day window for speed; not a copy of any production model.

Usage (authoring only -- see PORTABILITY above):
    python3 build_smoke_models.py
"""
import os
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
FIXTURES = HERE / "fixtures"
OUT_DIR = HERE / "generated"
REPO_ROOT = HERE.parent.parent  # tests/resource_smoke -> tests -> <repo root>
RESOURCES_DIR = REPO_ROOT / "resources"

SOURCE_OHQ = (
    Path("/home/arash/Dropbox/Environmental Flow paper/OHQ_Model/v2_development/")
    / "cli_build" / "Nezamabad_riparian_hru.ohq"
)

KEEP_SUBBASINS = ["9"]
KEEP_REACHES = {"Reach_19", "Reach_20"}
DAYS = 10

VARIANTS = {
    # hillslope_to_near_stream's flow is proportional to width/length (see
    # UC_groundwater_link's flow expression: ...*width*(depth.s+depth.e)/
    # length). Setting width_scale == length_scale to the SAME value (an
    # earlier version used 0.8/0.8) leaves that ratio, and therefore flow,
    # UNCHANGED -- it proves nothing about either parameter. Three variants
    # below isolate each one: a 1/1 baseline, width_scale=0.5 alone (flow
    # should roughly halve), length_scale=2.0 alone (flow should also
    # roughly halve, from the denominator). Each tests one factor at a time.
    "smoke_default_baseline": dict(composite_type=None),
    "smoke_default_width_scale_half": dict(composite_type=None, width_scale=0.5),
    "smoke_default_length_scale_double": dict(composite_type=None, length_scale=2.0),
    # Default initial conditions leave the gradient upward-favoring for the
    # entire 10-day window, so recharge_flow is identically 0 throughout --
    # that only proves "never negative" (trivially true by construction),
    # not that the permitted downward branch actually works. Wetting the
    # soil close to saturation (0.38, vs. theta_sat=0.4) and lowering the
    # water table 5m flips the gradient to downward-favoring for this
    # window (confirmed: recharge_flow ranges ~7.7M-316M m^3/day, nowhere
    # near 0), so this fixture now exercises the real, permitted branch.
    "smoke_oneway_recharge": dict(
        composite_type="Riparian_Urban_HRU_OneWayRecharge",
        initial_moisture_content=0.38, water_table_offset=5.0,
    ),
    "smoke_symmetric_k_recharge": dict(composite_type="Riparian_Urban_HRU_SymmetricKRecharge"),
}


def subbasin_of(line: str):
    m = re.search(r"Subbasin_(\d+)", line)
    return m.group(1) if m else None


def build(name: str, composite_type=None, width_scale=None, length_scale=None,
          initial_moisture_content=None, water_table_offset=None):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    assert SOURCE_OHQ.exists(), f"source model not found: {SOURCE_OHQ}"
    lines = SOURCE_OHQ.read_text().splitlines()

    kept = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("# Skipped invalid self-link") or stripped.startswith(
            "# GIS junction used as topology-only node"
        ):
            continue
        if not stripped.startswith("create") and not stripped.startswith("setvalue"):
            kept.append(line)
            continue
        sb = subbasin_of(line)
        if sb is not None and sb not in KEEP_SUBBASINS:
            continue
        if stripped.startswith("create block;type=Trapezoidal Channel Segment"):
            m = re.search(r"name=(Reach_\d+),", line)
            if m and m.group(1) not in KEEP_REACHES:
                continue
        if stripped.startswith("create link;from=Reach_"):
            if stripped.startswith("create link;from=Reach_19,to=Reach_20,"):
                kept.append(line)
                continue
            if re.match(r"create link;from=Reach_\d+,to=Outlet,", stripped):
                m = re.match(r"create link;from=(Reach_\d+),to=Outlet,(.*)", stripped)
                kept.append(
                    f"create link;from=Reach_20,to=Outlet,{m.group(2)}"
                    .replace(f"name={m.group(1)} to Outlet", "name=Reach_20 to Outlet")
                )
                continue
            continue
        kept.append(line)

    text = "\n".join(kept)

    # loadtemplate/addtemplate lines carry whatever absolute resources/
    # path was baked in by the original GIStoOHQ CLI run (machine-specific).
    # Replace with a path relative to OUT_DIR -- confirmed the console
    # resolves these correctly against the .ohq's own directory, same as
    # the forcing-file paths below.
    resources_rel = os.path.relpath(RESOURCES_DIR, OUT_DIR)
    text = re.sub(
        r"((?:load|add)template;\s*filename=)\S*?/resources/([A-Za-z0-9_.]+\.json)",
        rf"\1{resources_rel}/\2",
        text,
    )

    sb = "9"
    # Paths relative to OUT_DIR (the .ohq's own directory, which is where
    # the console resolves relative paths against by default) -- not
    # absolute, so the committed fixtures are portable across checkouts.
    rain_file = Path(os.path.relpath(FIXTURES / "rain_sub9.txt", OUT_DIR))
    temp_file = Path(os.path.relpath(FIXTURES / "temp_sub9.txt", OUT_DIR))
    et_file = Path(os.path.relpath(FIXTURES / "ET_1388_1398.txt", OUT_DIR))
    for f in (FIXTURES / "rain_sub9.txt", FIXTURES / "temp_sub9.txt", FIXTURES / "ET_1388_1398.txt"):
        assert f.exists(), f

    temp_source_name = f"AirTemperature_{sb}"
    temp_source_line = (
        f"create source;type=Air_Temperature,name={temp_source_name},"
        f"timeseries={temp_file}"
    )
    text = re.sub(
        rf"(create source;type=Liquid_Precipitation,name=LiquidRain_Subbasin_{sb},)",
        temp_source_line + "\n" + r"\1",
        text,
    )
    text = re.sub(
        rf"(name=(?:LiquidRain|Snowfall)_Subbasin_{sb},)"
        r"timeseries=\S+?,Temperature=\S+?,temperature_offset=-?[\d.]+",
        rf"\1timeseries={rain_file},Temperature={temp_file},"
        r"temperature_offset=0",
        text,
    )
    text = re.sub(
        rf"(name=Snowpack_Subbasin_{sb}_(?:Pervious|Impervious),[^\n]*?)"
        r"Temperature=AirTemperature,temperature_offset=-?[\d.]+",
        rf"\1Temperature={temp_source_name},temperature_offset=0",
        text,
    )
    text = text.replace(
        "/mnt/3rd900/Projects/GIStoOHQ/examples/Zarrineh/Nezamabad/outputs/"
        "upstream_calib_v1/ET_1388_1398.txt",
        str(et_file),
    )
    text = "\n".join(
        l for l in text.splitlines()
        if not l.startswith("create source;type=Air_Temperature,name=AirTemperature,")
    )

    if composite_type:
        text = text.replace(
            f"type=Riparian_Urban_HRU,name=Subbasin_{sb},",
            f"type={composite_type},name=Subbasin_{sb},",
        )
    if width_scale is not None or length_scale is not None or initial_moisture_content is not None:
        extra = ""
        if width_scale is not None:
            extra += f",hillslope_width_scale={width_scale:g}"
        if length_scale is not None:
            extra += f",hillslope_length_scale={length_scale:g}"
        if initial_moisture_content is not None:
            extra += f",initial_moisture_content={initial_moisture_content:g}"
        text = re.sub(
            rf"(create composite;type=\S+,name=Subbasin_{sb},[^\n]*?near_stream_K=1\[m/day\],)",
            rf"\1{extra.lstrip(',')},",
            text,
        )

    # initial_water_table_elevation offset: subtract from the subbasin's
    # existing literal value (same mechanism as
    # OHQ_Model/v2_development/structural_experiments.py's water_table_offset).
    if water_table_offset:
        def _wte_sub(m):
            return f"initial_water_table_elevation={float(m.group(1)) - water_table_offset:.3f}[m]"
        new_lines = []
        for line in text.splitlines():
            if f"name=Subbasin_{sb}," in line and "initial_water_table_elevation=" in line:
                line = re.sub(r"initial_water_table_elevation=([\d.]+)\[m\]", _wte_sub, line)
            new_lines.append(line)
        text = "\n".join(new_lines)

    text = text.replace(
        "setvalue; object=system, quantity=simulation_end_time, value=1",
        f"setvalue; object=system, quantity=simulation_end_time, value={DAYS}\n"
        f"setvalue; object=system, quantity=initial_time_step, value=0.01",
    )
    out_file_name = f"{name}_OHQ_output.txt"
    text = text.replace("Nezamabad_riparian_hru_OHQ_output.txt", out_file_name)
    text = text.replace(
        "Nezamabad_riparian_hru_observed.txt", f"{name}_observed.txt"
    )

    out_ohq = OUT_DIR / f"{name}.ohq"
    out_ohq.write_text(text)
    return out_ohq


def main():
    for name, kwargs in VARIANTS.items():
        out_ohq = build(name, **kwargs)
        print(f"Wrote {out_ohq}")


if __name__ == "__main__":
    main()
