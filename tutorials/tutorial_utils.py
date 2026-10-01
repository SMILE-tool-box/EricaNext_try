"""Shared helpers for the EricaNext tutorials.

Each tutorial notebook finds this file by walking up from its own directory,
so the notebooks can live at any depth under tutorials/.
"""
import os
import subprocess
import sys
from pathlib import Path

import numpy as np

TUTORIALS = Path(__file__).resolve().parent
REPO = TUTORIALS.parent
ERICANEXT_PY = REPO / "install" / "lib" / "python"
ANLNEXT_PY = Path(os.environ.get("ANLNEXT_INSTALL", "/usr/local")) / "lib" / "python"

# ---------------------------------------------------------------- data
# Where the data lives on the lab server. Copy it with rsync (see
# print_download_instructions); on the server itself it is used in place.
SERVER = "ramune"
SERVER_DIR = "/home/onoda/hikaru3_analysis2/sm2+_cal_rawdata"
PARAM_SUBDIR = "param/20190528"


def data_top():
    """Directory that holds <Period>_ETCC/ and param/.

    Order: $SMILE2P_DATA, ~/smile2p_data, the server directory (when running
    on the server).
    """
    env = os.environ.get("SMILE2P_DATA")
    if env:
        return Path(env).expanduser()
    local = Path("~/smile2p_data").expanduser()
    if local.is_dir() or not Path(SERVER_DIR).is_dir():
        return local
    return Path(SERVER_DIR)


def print_download_instructions(period="Per02"):
    print(f"""# 生データ({period})とパラメータを ~/smile2p_data にコピーする
mkdir -p ~/smile2p_data
rsync -av --progress <user>@{SERVER}:{SERVER_DIR}/{period}_ETCC ~/smile2p_data/
rsync -av --progress <user>@{SERVER}:{SERVER_DIR}/param ~/smile2p_data/""")


def check_data(period="Per02"):
    top = data_top()
    need = [top / f"{period}_ETCC", top / PARAM_SUBDIR]
    missing = [str(p) for p in need if not p.is_dir()]
    if missing:
        print_download_instructions(period)
        raise FileNotFoundError(
            f"データが見つかりません: {missing}\n"
            "上のコマンドでコピーするか、環境変数 SMILE2P_DATA にデータの場所を設定してください。")
    n_raw = len(list((top / f"{period}_ETCC").glob("*.raw")))
    print(f"data OK: {top}  ({period}: {n_raw} raw files)")
    return top


# ---------------------------------------------------------------- chain
CHAIN = ["ReadData", "CalcFADC", "TPC_Calibration", "HA_CenterGravity",
         "HA_Calc", "TOT_Skewness", "Reconstruct"]
RAW_CHAIN = ["ReadData", "RawTreeWriter"]


def default_parameters(period="Per02", drift_file=None):
    """Module parameters used by the SMILE_tool_box_v2.20 reference run."""
    top = data_top()
    param_dir = str(top / PARAM_SUBDIR)
    data_dir = str(top / f"{period}_ETCC")
    return {
        "ReadData": {"data_dir": data_dir, "reverse_TPC": True},
        "CalcFADC": {"base_window": 30.0, "threshold": 20.0},
        "TPC_Calibration": {"ParameterFile": param_dir + "/Calibration20190130.dat",
                            "Cal_Modify": 1.45305},
        "HA_CenterGravity": {"threshold": 10, "overflow": 4000},
        "HA_Calc": {"parameter_dir": param_dir,
                    "Flag_Modify_PMT_Gain": True,
                    "Modified_Parameter_Data_Dir": param_dir,
                    "SingleHit_Eth": 50.0, "SingleHit_ADCth": 1000.0},
        "TOT_Skewness": {"drift_velocity_file":
                         drift_file or data_dir + "/drift_velocity.dat"},
        "Reconstruct": {},
        "RawTreeWriter": {},
    }


def run_chain(output, period="Per02", n_events=10000, upto="Reconstruct",
              parameters=None, overwrite=False, modules=None, compression=-1):
    """Run ReadData -> ... -> `upto` (or the explicit `modules` list) in a
    separate python process.

    CalcFADC uses std::thread, which can crash a Jupyter kernel, so the
    chain always runs in a subprocess. `parameters` overrides
    default_parameters() per module; `compression` is SaveFile's ROOT
    compression setting (505 = zstd, recommended for raw trees).
    """
    output = Path(output).resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists() and not overwrite:
        print(f"{output.name} は既にあるので再実行しません(overwrite=True で作り直し)")
        return output
    params = default_parameters(period)
    for mod, p in (parameters or {}).items():
        params[mod].update(p)
    modules = modules or CHAIN[:CHAIN.index(upto) + 1]

    lines = ["import sys",
             f"sys.path.insert(0, {str(ANLNEXT_PY)!r})",
             f"sys.path.insert(0, {str(ERICANEXT_PY)!r})",
             "import anlnext", "import EricaPy as th",
             "a = anlnext.AnalysisChain()",
             f"a.chain(th.SaveFile); a.with_parameters({{'filename': {str(output)!r}, 'compression': {int(compression)}}})"]
    for m in modules:
        lines.append(f"a.chain(th.{m})")
        if params[m]:
            lines.append(f"a.with_parameters({params[m]!r})")
    lines.append(f"a.run({int(n_events)})")
    script = output.with_suffix(".run.py")
    script.write_text("\n".join(lines) + "\n")

    print(f"running {' -> '.join(modules)} on {period}, {n_events} events ...")
    res = subprocess.run([sys.executable, str(script)], capture_output=True, text=True)
    if res.returncode != 0:
        print(res.stdout[-2000:])
        print(res.stderr[-3000:])
        res.check_returncode()
    print("done:", output)
    return output


# ---------------------------------------------------------------- reading
def setup_pyroot():
    import ROOT
    ROOT.gROOT.SetBatch(True)
    return ROOT


def th1_to_numpy(h):
    n = h.GetNbinsX()
    edges = np.array([h.GetXaxis().GetBinLowEdge(i) for i in range(1, n + 2)])
    vals = np.array([h.GetBinContent(i) for i in range(1, n + 1)])
    return vals, edges


def th2_to_numpy(h):
    """Return (values[nx, ny], xedges, yedges) of a ROOT TH2."""
    nx, ny = h.GetNbinsX(), h.GetNbinsY()
    xe = np.array([h.GetXaxis().GetBinLowEdge(i) for i in range(1, nx + 2)])
    ye = np.array([h.GetYaxis().GetBinLowEdge(i) for i in range(1, ny + 2)])
    dtype = {"TH2S": np.int16, "TH2I": np.int32,
             "TH2F": np.float32, "TH2D": np.float64}.get(h.ClassName())
    if dtype is None:   # TH2C: GetArray() is a char*, read bin by bin
        vals = np.array([[h.GetBinContent(i, j) for j in range(1, ny + 1)]
                         for i in range(1, nx + 1)])
        return vals, xe, ye
    arr = np.frombuffer(h.GetArray(), dtype=dtype, count=(nx + 2) * (ny + 2))
    vals = arr.reshape(ny + 2, nx + 2)[1:-1, 1:-1].T.astype(float)
    return vals, xe, ye


def load_single_hit(path, branches=None, valid_only=True):
    """Read Reconstruct/single_hit into a dict of numpy arrays.

    valid_only=True keeps only err_flag == 0 rows (other rows carry the
    previous event's Eg/Ke/Pg, exactly like SMILE_tool_box's s2t).
    """
    ROOT = setup_pyroot()
    df = ROOT.RDataFrame("Reconstruct/single_hit", str(path))
    if valid_only:
        df = df.Filter("err_flag == 0")
    cols = branches or ["sevt_id", "err_flag", "Eg", "Ke", "Pg", "Pe_TRA", "Ve_TRA",
                        "Len_TRA", "Track_Min", "Track_Max", "alpha_kin",
                        "phi_kin", "psi_kin", "skewness_x", "skewness_y"]
    raw = df.AsNumpy(cols)
    out = {}
    for k, v in raw.items():
        if v.dtype == object:
            v = np.stack([np.asarray(x) for x in v]) if len(v) else np.zeros((0, 0))
        out[k] = v
    for k in ("Pe_TRA", "Ve_TRA"):
        if k in out and out[k].ndim == 2:
            out[k] = out[k][:, :3]
    return out


def get_reconstructed(period="Per02", n_events=50000):
    """Full-chain output shared by the analysis tutorials.

    Kept in tutorials/cache/, so the first notebook that needs it runs the
    chain and the others reuse the file.
    """
    out = TUTORIALS / "cache" / f"{period}_{n_events}.root"
    out.parent.mkdir(exist_ok=True)
    return run_chain(out, period=period, n_events=n_events)


def get_raw(period="Per02", n_events=5000):
    """ReadData -> RawTreeWriter output (RawTreeWriter/raw), cached in
    tutorials/cache/. Used to draw the per-module diagnostics."""
    out = TUTORIALS / "cache" / f"{period}_raw_{n_events}.root"
    return run_chain(out, period=period, n_events=n_events,
                     modules=RAW_CHAIN, compression=505)


def iter_raw(path, branches, n=None):
    """Yield one dict of numpy arrays per event of RawTreeWriter/raw."""
    ROOT = setup_pyroot()
    f = ROOT.TFile.Open(str(path))
    t = f.Get("RawTreeWriter/raw")
    t.SetBranchStatus("*", 0)
    for b in branches:
        t.SetBranchStatus(b, 1)
    n = t.GetEntries() if n is None else min(n, t.GetEntries())
    for i in range(n):
        t.GetEntry(i)
        yield {b: np.asarray(getattr(t, b)) for b in branches}


def raw_waveforms(path, n=None):
    """Yield (fadc_ch[k], waves[k, 512]) per event. fadc_ch: 0-11 anode,
    12-23 cathode; cathode waveforms are inverted as CalcFADC sees them."""
    for ev in iter_raw(path, ["fadc_ch", "fadc_wave"], n):
        ch = ev["fadc_ch"].astype(int)
        yield ch, ev["fadc_wave"].astype(float).reshape(len(ch), 512)


def raw_ha(path, n=None):
    """HA ADC (masked 12 bit) and error bits: arrays [event, module, 24]."""
    ROOT = setup_pyroot()
    d = ROOT.RDataFrame("RawTreeWriter/raw", str(path))
    if n is not None:
        d = d.Range(n)
    d = d.Define("ha_err_i", "ROOT::RVecI(ha_err.begin(), ha_err.end())")
    a = d.AsNumpy(["ha_adc", "ha_err_i"])
    adc = np.stack([np.asarray(v) for v in a["ha_adc"]]).astype(float)
    err = np.stack([np.asarray(v) for v in a["ha_err_i"]]).astype(bool)
    return adc.reshape(len(adc), -1, 24), err.reshape(len(err), -1, 24)


def raw_info(path):
    ROOT = setup_pyroot()
    a = ROOT.RDataFrame("RawTreeWriter/run_info", str(path)).AsNumpy(
        ["anode_ids", "cathode_ids", "ha_ids"])
    return {k: [int(x) for x in v[0]] for k, v in a.items()}


# ---------------------------------------------------------------- selection
MC2 = 510.9989461   # keV, SMLib Compton_common::mass_e


def setcrit(ev):
    """Event selection of the production analysis (SetCrit in
    s2p_chk_std2_Erica_v2_1.cc), applied to load_single_hit() output.

    Returns an ordered dict {name: boolean mask} of the individual cuts and
    the derived quantities used by them.
    """
    Eg, Ke, Len = ev["Eg"], ev["Ke"], ev["Len_TRA"]
    Pg, Pe, Ve = ev["Pg"], ev["Pe_TRA"], ev["Ve_TRA"]
    tmin, tmax = ev["Track_Min"], ev["Track_Max"]

    g = Pg - Pe
    g_unit = g / np.linalg.norm(g, axis=1, keepdims=True)
    e_unit = Ve / np.linalg.norm(Ve, axis=1, keepdims=True)
    cos_geo = np.sum(g_unit * e_unit, axis=1)
    dcos = cos_geo - np.cos(np.radians(ev["alpha_kin"]))
    ke, eg = Ke / 1000., Eg / 1000.   # MeV

    with np.errstate(invalid="ignore"):
        cuts = {
            "Eg": (eg >= 0.15) & (eg <= 2.1),
            "fiducial": ((-153.6 + 5. < tmin[:, 0]) & (tmax[:, 0] < -5. + 153.6)
                         & (-153.6 + 5. < tmin[:, 1]) & (tmax[:, 1] < -5. + 153.6)
                         & (27.0 + 5. < tmin[:, 2]) & (tmax[:, 2] < -5. + 332.0)),
            "dEdx": ((Len < 1975.9 * np.power(ke, 1.4) + 7.)
                     & (Len > 913.9 * np.power(ke, 2.0) + 2.2)
                     & (Len < 270.)),
            "dcos_alpha": (-0.5 <= dcos) & (dcos <= 0.5),
        }
    derived = {"g_unit": g_unit, "e_unit": e_unit, "cos_geo": cos_geo, "dcos": dcos}
    return cuts, derived
