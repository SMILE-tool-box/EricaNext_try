#!/usr/bin/env python3
"""Check a drift_velocity.dat made by tutorials_caldata/4_MakeCalibration/DriftVelocity.ipynb
against the one of the data (made with SMILE_tool_box / drift_speed_s2p_v3.C).

    python3 check_drift_velocity.py                      # cache/drift_velocity_Per02.dat vs the Per02 config's file
    python3 check_drift_velocity.py made.dat ref.dat --tolerance 1e-3

Exit status 1 when the offsets differ or the largest relative velocity difference exceeds
--tolerance (default 1e-3). Status of the Per02 check: offset 85 = 85, largest difference
1.07e-4 mm/clock (2.8e-4 relative), see docs/calibration_log.md.
"""
import argparse
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, "..")
sys.path.insert(0, os.path.join(REPO, "config"))
import ericanext_config as ec


def read(path):
    values = open(path).read().split()
    return float(values[0]), np.array(values[1:], float).reshape(-1, 2)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("made", nargs="?", default=os.path.join(REPO, "tutorials_caldata", "cache", "drift_velocity_Per02.dat"))
    ap.add_argument("reference", nargs="?",
                    default=ec.load("smile2p_20190701_Per02")["parameters"]["TOT_Skewness"]["drift_velocity_file"])
    ap.add_argument("--tolerance", type=float, default=1e-3, help="largest allowed relative velocity difference")
    args = ap.parse_args()

    off_m, rows_m = read(args.made)
    off_r, rows_r = read(args.reference)
    n = min(len(rows_m), len(rows_r))
    if len(rows_m) != len(rows_r):
        print(f"rows differ: {len(rows_m)} (made) vs {len(rows_r)} (reference); comparing the first {n}")
    if not np.allclose(rows_m[:n, 0], rows_r[:n, 0], rtol=0, atol=1e-4):
        print("time columns differ")
    diff = np.abs(rows_m[:n, 1] - rows_r[:n, 1])
    rel = diff / rows_r[:n, 1]
    print(f"made     : {args.made}")
    print(f"reference: {args.reference}")
    print(f"offset   : {off_m:g} (made) vs {off_r:g} (reference)")
    print(f"velocity : largest difference {diff.max():.3e} mm/clock, relative {rel.max():.3e} (row {rel.argmax()})")
    ok = off_m == off_r and rel.max() <= args.tolerance
    print("OK" if ok else f"NG (tolerance {args.tolerance:g})")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
