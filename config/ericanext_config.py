"""Run configurations of the EricaNext chain (config/*.json).

    import ericanext_config as ec
    cfg = ec.load("smile2p_20190701_Per02")
    cfg["chain"]                     # module order
    cfg["parameters"]["CalcFADC"]    # {"base_window": 30.0, "threshold": 20.0}

Keys starting with "_" are comments and are dropped by load(); in strings
"{data_top}" is replaced by data_top(), "{tpc_caldata}" by tpc_caldata() and
"{sim_data}" by sim_data().

Simulation configurations are in config/simulation/*.json (load_sim()); there
"{sim_data}" is replaced by sim_data(), "{parma_flux}" by parma_flux(), "{home}" by
the home directory and "{data_top}" by data_top().
"""
import json
import os
from pathlib import Path

CONFIG_DIR = Path(__file__).resolve().parent

# The data of EricaNext_try (on the lab NAS). Layout:
#   calibration_data/sm2+_cal_rawdata/<date>_<Period>/   raw data (uTPC_*.raw, HA*.raw) and drift_velocity.dat
#   calibration_data/param/<date>/                        calibration files (Calibration*.dat, MDF, Cal_HA, HAs.pos, ...)
#   TPC_caldata/<date>/<Period>_TPC/                      TPC-only calibration runs (Ba-133, 2017-12-19/20)
#   simulation_data/parameters/smile2p_sim_data/          simulation data (SMILE_tool_box_v2.20/data + emulator_param)
#   simulation_data/parma_flux/                           PARMA cosmic-ray flux files (parma_flux_d<depth>_g10_thermal.root)
# To use a copy elsewhere, keep the same layout and set $SMILE2P_DATA to the directory that holds calibration_data/ etc.
# (= SMILE2+); the directory above it (a copy of the whole EricaNext_try_data) works too.
DATA_DIR = "/mnt/nas_eureka/onoda/EricaNext_try_data/SMILE2+"


def data_root():
    """Top of the data directory (the one with calibration_data/ etc. directly inside, = SMILE2+ on the NAS):
    $SMILE2P_DATA, else DATA_DIR (the NAS). If $SMILE2P_DATA has no calibration_data/ but a SMILE2+/ (a copy of the
    whole EricaNext_try_data directory), SMILE2+/ is used."""
    root = Path(os.environ.get("SMILE2P_DATA", DATA_DIR)).expanduser()
    if not (root / "calibration_data").is_dir() and (root / "SMILE2+" / "calibration_data").is_dir():
        root = root / "SMILE2+"
    return root


def data_top():
    """{data_top}: directory that holds sm2+_cal_rawdata/ and param/."""
    return data_root() / "calibration_data"


def tpc_caldata():
    """{tpc_caldata}: directory of the TPC-only calibration runs (<date>/<Period>_TPC/)."""
    return data_root() / "TPC_caldata"


def sim_data():
    """{sim_data}: directory of the simulation data (SMILE_tool_box_v2.20/data and emulator_param/)."""
    return data_root() / "simulation_data" / "parameters" / "smile2p_sim_data"


def parma_flux():
    """{parma_flux}: directory of the PARMA cosmic-ray flux files."""
    return data_root() / "simulation_data" / "parma_flux"


def available():
    return sorted(p.stem for p in CONFIG_DIR.glob("*.json"))


SIM_CONFIG_DIR = CONFIG_DIR / "simulation"


def available_sim():
    return sorted(p.stem for p in SIM_CONFIG_DIR.glob("*.json"))


def _resolve(obj, subs):
    if isinstance(obj, dict):
        return {k: _resolve(v, subs) for k, v in obj.items() if not k.startswith("_")}
    if isinstance(obj, list):
        return [_resolve(v, subs) for v in obj]
    if isinstance(obj, str):
        for key, value in subs.items():
            obj = obj.replace(key, str(value))
        return obj
    return obj


def load(name, data_top_dir=None):
    """Load config/<name>.json (or a path to a .json file)."""
    path = Path(name)
    if path.suffix != ".json":
        path = CONFIG_DIR / f"{name}.json"
    with open(path) as fp:
        raw = json.load(fp)
    return _resolve(raw, {"{data_top}": Path(data_top_dir) if data_top_dir else data_top(),
                          "{tpc_caldata}": tpc_caldata(), "{sim_data}": sim_data()})


def load_sim(name):
    """Load config/simulation/<name>.json (or a path to a .json file)."""
    path = Path(name)
    if path.suffix != ".json":
        path = SIM_CONFIG_DIR / f"{name}.json"
    with open(path) as fp:
        raw = json.load(fp)
    return _resolve(raw, {"{sim_data}": sim_data(), "{parma_flux}": parma_flux(), "{home}": Path.home(),
                          "{data_top}": data_top()})
