# Full-run comparison aligned by event ID (the reference's anode_id is a
# 24-bit counter; the reference also drops 26 events after the first wrap and
# the last event, which are removed from ours before comparing row by row).
import sys, ROOT, numpy as np
OURS = sys.argv[1]
REF = '/home/onoda/hikaru3_analysis2/sm2+_cal_rootdata/20190701_Per02/haboc_tmp_0.root'
pairs = [('err_flag','err_flag',1),('Eg','Eg',1),('Ke','Ee',1),('Pg','Pg',3),('phi_kin','phi_kin',1),('psi_kin','psi_kin',1),
         ('alpha_kin','alpha_kin.alp_kin',1),('TOT_Area','TOT_Area',2),('Pe_TRA','Pe_TRA',6),('Ve_TRA','Ve_TRA',6),('Len_TRA','Len_TRA',1),
         ('Ve_TRA_fit_status','Ve_TRA_fit_status',2),('Ve_TRA_grad_err','Ve_TRA_grad_err',2),('Track_Min','Track_Min',3),
         ('Track_Max','Track_Max',3),('skewness_x','skewness_x',1),('skewness_y','skewness_y',1),
         ('HA_Pixel','HA_Pixel',8),('Hit_Flag_Num','Hit_Flag_Num',1)]
def load(path, tree, cols, extra=None):
    df = ROOT.RDataFrame(tree, path)
    if extra: df = df.Define(*extra)
    d = df.AsNumpy(cols)
    out = {}
    for k, v in d.items():
        out[k] = np.stack([np.asarray(x, dtype=float) for x in v]) if v.dtype == object else v.astype(float)
    return out
o = load(OURS, "Reconstruct/single_hit", [p[0] for p in pairs] + ["event_id"])
r = load(REF, "Reconstruct/s2t", [p[1] for p in pairs], ("aid", "(double)anode_id[0]"))
r["aid"] = load(REF, "Reconstruct/s2t", ["aid"], ("aid", "(double)anode_id[0]"))["aid"]
er = r["aid"].astype(np.int64)
wraps = np.concatenate([[0], np.cumsum(np.diff(er) < -(1 << 23))]); eru = er + (wraps << 24)
keep = np.isin(o["event_id"].astype(np.int64), eru)
print("ours rows", len(keep), "kept", keep.sum(), "ref rows", len(er))
assert np.array_equal(o["event_id"][keep].astype(np.int64), eru)
good = r["err_flag"] == 0
for a, b, sz in pairs:
    x = o[a][keep]; y = r[b]
    if x.ndim == 1 and y.ndim == 2: y = y[:, 0]
    if x.ndim == 2: x, y = x[:, :sz], y[:, :sz]
    same = (x == y) | (np.isnan(x) & np.isnan(y))
    near = same | (np.abs(x - y) <= 4e-14 * np.maximum(1, np.abs(y)))
    if x.ndim == 2: same, near = same.all(1), near.all(1)
    n = len(same)
    print(f"{a:18s} exact {same.sum():7d}  ulp {(near & ~same).sum():5d}  other {(~near).sum():6d}"
          f"   | err_flag==0 rows: other {(~near & good).sum():6d} / {good.sum()}")

# ---- details
def rows(a, b, sz):
    x = o[a][keep]; y = r[b]
    if x.ndim == 1 and y.ndim == 2: y = y[:, 0]
    if x.ndim == 2: x, y = x[:, :sz], y[:, :sz]
    bad = ~((x == y) | (np.abs(x - y) <= 4e-14 * np.maximum(1, np.abs(y))))
    return np.nonzero(bad.any(1) if bad.ndim == 2 else bad)[0], x, y
i, x, y = rows("Eg", "Eg", 1); print("Eg mismatch rows", i, "removed-block boundary row", np.argmax(~keep[:200000][131600:]) + 131600)
i, x, y = rows("Ve_TRA_fit_status", "Ve_TRA_fit_status", 2)
v = i[good[i]]; print("fit_status valid-row examples (ours, ref):", [(x[k].tolist(), y[k].tolist()) for k in v[:6]])
i, x, y = rows("Track_Min", "Track_Min", 3)
v = i[good[i]]; print("Track_Min valid-row examples (ours, ref):", [(np.round(x[k],2).tolist(), np.round(y[k],2).tolist()) for k in v[:4]])
x = o["Ve_TRA"][keep][:, :3]; y = r["Ve_TRA"][:, :3]; m = good
rel = np.abs(x[m] - y[m]).max(1) / np.maximum(1e-300, np.abs(y[m]).max(1))
print("Ve_TRA rel diff (valid rows): median %.1e  >1e-6: %d  >1e-3: %d" % (np.median(rel[rel > 0]), (rel > 1e-6).sum(), (rel > 1e-3).sum()))
i, x, y = rows("Track_Min", "Track_Min", 3)
v = i[good[i]]
prev_same = [np.allclose(y[k], y[k-1]) for k in v]
oursmax = o["Track_Max"][keep][:, :3]; refmax = r["Track_Max"][:, :3]
print("Track_Min mismatch valid rows:", len(v), " ref value == ref previous row:", sum(prev_same))
print("ours Track_Max at those rows equals Track_Min?", sum(np.allclose(oursmax[k], x[k]) for k in v))
