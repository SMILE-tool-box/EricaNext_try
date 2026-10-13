"""Run configurations of the EricaNext chain (config/*.json).

    import ericanext_config as ec
    cfg = ec.load("smile2p_20190701_Per02")
    cfg["chain"]                     # module order
    cfg["parameters"]["CalcFADC"]    # {"base_window": 30.0, "threshold": 20.0}

Keys starting with "_" are comments and are dropped by load(); "{data_top}"
in strings is replaced by data_top().

Simulation configurations are in config/simulation/*.json (load_sim()); there
"{sim_data}" is replaced by sim_data() and "{home}" by the home directory.
"""
import json
import os
from pathlib import Path

CONFIG_DIR = Path(__file__).resolve().parent

# Where the data lives on the lab server (copy it with rsync, or use it in
# place when running on the server).
SERVER = "ramune"
SERVER_DIR = "/home/onoda/hikaru3_analysis2/sm2+_cal_rawdata"


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


def available():
    return sorted(p.stem for p in CONFIG_DIR.glob("*.json"))


SIM_CONFIG_DIR = CONFIG_DIR / "simulation"


def sim_data():
    """Directory of the simulation data (SMILE_tool_box_v2.20/data): $SMILE2P_SIM_DATA, else ~/smile2p_sim_data."""
    return Path(os.environ.get("SMILE2P_SIM_DATA", "~/smile2p_sim_data")).expanduser()


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
    return _resolve(raw, {"{data_top}": Path(data_top_dir) if data_top_dir else data_top()})


def load_sim(name):
    """Load config/simulation/<name>.json (or a path to a .json file)."""
    path = Path(name)
    if path.suffix != ".json":
        path = SIM_CONFIG_DIR / f"{name}.json"
    with open(path) as fp:
        raw = json.load(fp)
    return _resolve(raw, {"{sim_data}": sim_data(), "{home}": Path.home()})
