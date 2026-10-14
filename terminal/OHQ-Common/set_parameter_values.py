#!/usr/bin/env python3
# OpenHydroQual - Environmental Modeling Platform
# Copyright (C) 2025 Arash Massoudieh
#
# This file is part of OpenHydroQual, licensed under the GNU Affero General
# Public License, version 3 or later.

"""Write calibrated parameter values into the `value=` of a model's parameters.

LM starts from each parameter's `value=` and MCMC chains start there, so after
a GA or LM run the estimates have to be copied into the model. This does it.

    set_parameter_values.py model.ohq new_model.ohq --from work/GA_output.txt
    set_parameter_values.py model.ohq new_model.ohq --from work/LM_covariance.txt
    set_parameter_values.py model.ohq new_model.ohq --from work/posterior_percentiles.txt
    set_parameter_values.py model.ohq new_model.ohq p_K=12.5 p_n=0.3

--from takes the best individual of the last generation (GA_output.txt), the
estimates (LM_covariance.txt) or the posterior medians (posterior_percentiles.txt).
Pairs given on the command line are applied after, so they win. With
--show the values are only printed.
"""

import argparse
import csv
import io
import re
import sys


def from_ga(text):
    text = text.split("Final Enhancements")[0]
    parts = re.split(r"^Generation: (\d+)\n", text, flags=re.M)
    if len(parts) < 3:
        sys.exit("no complete generation in the GA output")
    rows = list(csv.DictReader(io.StringIO(parts[-1]), skipinitialspace=True))
    rows = [r for r in rows if float(r["neg_log_likelihood"]) < 1e17]
    if not rows:
        sys.exit("every individual of the last generation failed to solve")
    best = min(rows, key=lambda r: float(r["neg_log_likelihood"]))
    names = list(rows[0])[1:list(rows[0]).index("neg_log_likelihood")]
    return {n: best[n] for n in names}


def from_lm(text):
    values, table = {}, False
    for line in text.splitlines():
        cells = [c.strip() for c in line.split(",")]
        if cells[0] == "parameter":
            table = True
        elif table and len(cells) >= 3 and cells[0] and not line.startswith("#"):
            values[cells[0]] = cells[2]
        elif table:
            break
    return values


def from_mcmc(text):
    rows = [line.split("\t") for line in text.splitlines() if line.strip()]
    header = rows[0]
    for row in rows[1:]:
        if row[0] == "0.5":
            return dict(zip(header[1:], row[1:]))
    sys.exit("no 0.5 (median) row in the posterior percentiles")


def read_results(path):
    text = open(path).read()
    if "Generation:" in text:
        return from_ga(text)
    if "Levenberg-Marquardt parameter covariance" in text:
        return from_lm(text)
    if text.startswith("RowLabel"):
        return from_mcmc(text)
    sys.exit(f"{path}: not a GA_output, LM_covariance or posterior_percentiles file")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("model")
    ap.add_argument("output", nargs="?")
    ap.add_argument("pairs", nargs="*", metavar="name=value")
    ap.add_argument("--from", dest="results")
    ap.add_argument("--show", action="store_true", help="print the values, write nothing")
    a = ap.parse_intermixed_args()

    values = read_results(a.results) if a.results else {}
    pairs = list(a.pairs)
    if a.output and "=" in a.output:          # no output file, only pairs
        pairs.insert(0, a.output)
        a.output = None
    values.update(p.split("=", 1) for p in pairs)
    if not values:
        sys.exit("nothing to set: give --from and/or name=value pairs")

    for name, v in values.items():
        print(f"{name} = {v}")
    if a.show:
        return
    if not a.output:
        sys.exit("give an output file (the input model is never overwritten)")

    remaining = dict(values)
    out = []
    for line in open(a.model):
        m = re.match(r"create parameter;.*\bname=([^,\n]+)", line)
        if m and m.group(1).strip() in remaining:
            line = re.sub(r"\bvalue=[^,\n]*", "value=" + remaining.pop(m.group(1).strip()), line)
        out.append(line)
    if remaining:
        sys.exit("not parameters of the model: " + ", ".join(remaining))
    open(a.output, "w").writelines(out)
    print(f"wrote {a.output}")


if __name__ == "__main__":
    main()
