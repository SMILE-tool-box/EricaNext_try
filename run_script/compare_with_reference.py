#!/usr/bin/env python3
"""Compare an EricaNext output with the SMILE_tool_box reference, event by event.

    python3 compare_with_reference.py per02_out.root
    python3 compare_with_reference.py per02_out.root --config smile2p_20190701_Per02 --ref /path/to/haboc_tmp_0.root

Rows are matched by event ID: the reference's anode_id is a 24-bit counter
(unwrapped here), and events missing from the reference (it drops 26 events
after the first counter wrap and the last event of the run) are removed from
ours before the branches are compared row by row.

For each branch it prints the number of rows that are bit-identical, that
differ only in the last bits (|diff| <= 4e-14 relative), and that differ more;
the last column counts the latter among the err_flag == 0 rows only.
"""
import argparse
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "config"))
import ericanext_config as ec
import ROOT

# (our branch, reference branch, number of elements compared)
PAIRS = [('err_flag', 'err_flag', 1), ('Eg', 'Eg', 1), ('Ke', 'Ee', 1), ('Pg', 'Pg', 3),
         ('phi_kin', 'phi_kin', 1), ('psi_kin', 'psi_kin', 1), ('alpha_kin', 'alpha_kin.alp_kin', 1),
         ('TOT_Area', 'TOT_Area', 2), ('Pe_TRA', 'Pe_TRA', 6), ('Ve_TRA', 'Ve_TRA', 6),
         ('Len_TRA', 'Len_TRA', 1), ('Ve_TRA_fit_status', 'Ve_TRA_fit_status', 2),
         ('Ve_TRA_grad_err', 'Ve_TRA_grad_err', 2), ('Track_Min', 'Track_Min', 3),
         ('Track_Max', 'Track_Max', 3), ('skewness_x', 'skewness_x', 1), ('skewness_y', 'skewness_y', 1),
         ('HA_Pixel', 'HA_Pixel', 8), ('Hit_Flag_Num', 'Hit_Flag_Num', 1)]


def load(path, tree, cols, define=None):
    df = ROOT.RDataFrame(tree, path)
    if define:
        df = df.Define(*define)
    d = df.AsNumpy(cols)
    return {k: (np.stack([np.asarray(x, dtype=float) for x in v]) if v.dtype == object else v.astype(float))
            for k, v in d.items()}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("output", help="EricaNext output ROOT file (Reconstruct/single_hit)")
    ap.add_argument("--config", default="smile2p_20190701_Per02",
                    help="config whose reference.root is used (default: %(default)s)")
    ap.add_argument("--ref", help="reference ROOT file (overrides the config)")
    args = ap.parse_args()
    ref = args.ref or ec.load(args.config)["reference"]["root"]

    o = load(args.output, "Reconstruct/single_hit", [p[0] for p in PAIRS] + ["event_id"])
    r = load(ref, "Reconstruct/s2t", [p[1] for p in PAIRS] + ["aid"], ("aid", "(double)anode_id[0]"))
    er = r["aid"].astype(np.int64)
    wraps = np.concatenate([[0], np.cumsum(np.diff(er) < -(1 << 23))])
    eru = er + (wraps << 24)                                    # unwrapped 24-bit event ID
    eo = o["event_id"].astype(np.int64)
    keep = np.isin(eo, eru)
    print(f"ours: {len(eo)} rows, reference: {len(er)} rows, only in ours: {(~keep).sum()}, "
          f"only in reference: {(~np.isin(eru, eo)).sum()}")
    if not np.array_equal(eo[keep], eru):
        sys.exit("event order differs between the files; cannot compare row by row")

    good = r["err_flag"] == 0
    print(f"{'branch':18s} {'exact':>8s} {'ulp':>6s} {'other':>7s}   other with err_flag==0 (of {good.sum()})")
    for a, b, sz in PAIRS:
        x, y = o[a][keep], r[b]
        if x.ndim == 1 and y.ndim == 2:
            y = y[:, 0]
        if x.ndim == 2:
            x, y = x[:, :sz], y[:, :sz]
        same = (x == y) | (np.isnan(x) & np.isnan(y))
        near = same | (np.abs(x - y) <= 4e-14 * np.maximum(1, np.abs(y)))
        if x.ndim == 2:
            same, near = same.all(1), near.all(1)
        print(f"{a:18s} {same.sum():8d} {(near & ~same).sum():6d} {(~near).sum():7d}   {(~near & good).sum():7d}")


if __name__ == "__main__":
    main()
