"""Compare EricaNext ParmaCR runs (EdepPickUp) with the reference SMILE_tool_box run (d10, v1..v20).
Usage: python3 compare_parma_cr_with_reference.py "<edep files glob>" <n_generated> <out.png>"""
import glob, re, sys, numpy as np, ROOT
import matplotlib; matplotlib.use("Agg"); import matplotlib.pyplot as plt
REF = "/home/onoda/hikaru3_analysis2/BGsim/thermal_neutron_g10/simulation/d10"
ours, n_ours, out = sys.argv[1], int(sys.argv[2]), sys.argv[3]

# reference: counters and collected events (tpc && psa, DAQ_Mode 3)
gen = col = trg = 0; ref = {k: [] for k in ("SrcEng", "E_tpc", "E_psa", "E_anti")}
for v in sorted(glob.glob(REF + "/v*")):
    txt = open(v + "/ETCCcount.scl").read()
    gen += int(re.search(r"All Eevent\s*:\s*(\d+)", txt).group(1)); trg += int(re.search(r"Triggered\s*:\s*(\d+)", txt).group(1))
    a = ROOT.RDataFrame("EmuWrite2p/sim", v + "/haboc_etcc_emulator.root").AsNumpy(list(ref))
    col += len(a["E_tpc"])
    for k in ref: ref[k].append(np.asarray(a[k], float))
ref = {k: np.concatenate(v) for k, v in ref.items()}

d = ROOT.RDataFrame("edep", ours).Filter("e_tpc > 0 && e_scinti > 0")
a = d.AsNumpy(["primary_energy", "e_tpc", "e_scinti", "e_anti"])
my = {"SrcEng": a["primary_energy"], "E_tpc": a["e_tpc"], "E_psa": a["e_scinti"], "E_anti": a["e_anti"]}
my = {k: np.asarray(v, float) for k, v in my.items()}
mcol, mtrg = len(my["E_tpc"]), int(np.sum(my["E_anti"] == 0))

def rate(k, n): return f"{k/n:.4e} +- {np.sqrt(k)/n:.1e}"
print(f"reference: generated {gen}, collected(tpc&&psa) {col} ({rate(col, gen)}), triggered {trg} ({rate(trg, gen)})")
print(f"EricaNext: generated {n_ours}, tpc&&psa {mcol} ({rate(mcol, n_ours)}), triggered {mtrg} ({rate(mtrg, n_ours)})")
r1, r2 = mcol / n_ours / (col / gen), mtrg / n_ours / (trg / gen)
print(f"ratio EricaNext/reference: tpc&&psa {r1:.3f} +- {r1/np.sqrt(mcol):.3f}, triggered {r2:.3f} +- {r2/np.sqrt(mtrg):.3f}")
print(f"anti fraction among tpc&&psa: reference {np.mean(ref['E_anti']>0):.4f}, EricaNext {np.mean(my['E_anti']>0):.4f}")
for k in ("SrcEng", "E_tpc", "E_psa"):
    ks = ROOT.TMath.KolmogorovTest(len(ref[k]), np.sort(ref[k]), len(my[k]), np.sort(my[k]), "")
    print(f"{k:7s} median ref {np.median(ref[k]):.4g}  ours {np.median(my[k]):.4g}  KS p = {ks:.3g}")

fig, ax = plt.subplots(1, 3, figsize=(15, 4.2))
spec = [("SrcEng", "primary energy [keV]", np.logspace(-6, 1.1, 60)),
        ("E_tpc", "TPC deposit [keV]", np.logspace(-3, 3.2, 60)),
        ("E_psa", "scintillator deposit [keV]", np.logspace(-2, 4.4, 60))]
for x, (k, lab, b) in zip(ax, spec):
    x.hist(ref[k], bins=b, histtype="step", density=True, label=f"SMILE_tool_box (N={col})")
    x.hist(my[k], bins=b, histtype="step", density=True, label=f"EricaNext (N={mcol})")
    x.set_xscale("log"); x.set_yscale("log"); x.set_xlabel(lab); x.set_ylabel("normalized counts [/keV]")
ax[0].legend(fontsize=8)
fig.suptitle("PARMA thermal neutrons d10 g10, flight, TPC && scintillator events")
fig.tight_layout(); fig.savefig(out, dpi=110)
