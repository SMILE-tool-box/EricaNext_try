#!/usr/bin/env python3
"""Run the EricaNext simulation (Geant4 via anlgeant4) with a configuration from
config/simulation/*.json (environment: ~/opt/setup_ericanext.sh, sourced in ~/.bashrc).

    python3 run_sim.py smile2p_flight_parma_thermal_neutron_d10 -n 1000000 -o out.root --seed 1
    python3 run_sim.py my_setting.json -n 1000 -o test.root --set physics.em_msc=urban
    python3 run_sim.py smile2p_flight_parma_thermal_neutron_d10_raw -n 100000 -o edep.root --raw-dir rawdata

The config has the sections environment, detector, physics, source, geant4, pickup and
(optional) response (see config/README.md). The chain is
    detector -> physics -> source -> geant4 (Geant4Body) -> StandardUserActionAssembly -> pickup...
    -> response... (SimTrigger -> TPCEmulator -> HAEmulator -> RawWriter2p: raw files like measured data)
Modules are looked up in EricaSimPy, then anlgeant4.
"""
import argparse
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, "..")
sys.path.insert(0, os.path.join(REPO, "simulation_source", "install", "lib", "python"))
sys.path.insert(0, os.path.join(REPO, "config"))
import ericanext_config as ec

SECTIONS = ("detector", "physics", "source", "geant4")


def module_class(name):
    import anlgeant4
    import EricaSimPy
    for package in (EricaSimPy, anlgeant4):
        if hasattr(package, name):
            return getattr(package, name)
    sys.exit(f"run_sim.py: no module {name} in EricaSimPy or anlgeant4")


def is_vector3(value):
    return isinstance(value, list) and len(value) == 3 and all(isinstance(x, (int, float)) for x in value)


def add_module(chain, entry):
    chain.chain(module_class(entry["module"]))
    params = entry.get("parameters", {})
    plain = {k: v for k, v in params.items() if not is_vector3(v)}
    if plain:
        chain.with_parameters(plain)
    # the Python chain has no list -> 3-vector conversion: set them directly
    for key, value in params.items():
        if is_vector3(value):
            chain.with_setter(lambda m, k=key, v=value: m.set_parameter(k, *map(float, v)))


def apply_override(cfg, text):
    """--set section.key=value (value parsed as JSON when possible), e.g. physics.em_msc=urban."""
    path, value = text.split("=", 1)
    section, key = path.split(".", 1)
    try:
        value = json.loads(value)
    except json.JSONDecodeError:
        pass
    if section == "environment":
        cfg.setdefault("environment", {})[key] = value
    elif section in ("pickup", "response"):
        # pickup.key / response.Module.key
        if section == "response":
            module, key = key.split(".", 1)
            entry = next(e for e in cfg["response"] if e["module"] == module)
        else:
            entry = cfg["pickup"][0]
        entry.setdefault("parameters", {})[key] = value
    else:
        cfg[section]["parameters"][key] = value


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("config", help=f"config name or .json path ({', '.join(ec.available_sim())})")
    ap.add_argument("-n", "--n-events", type=int, required=True, help="number of primary events")
    ap.add_argument("-o", "--output", help="output ROOT file of the first pickup module")
    ap.add_argument("--seed", type=int, help="random seed (geant4.random_seed; the response modules use seed*100 + 1, + 2, ...)")
    ap.add_argument("--raw-dir", help="directory of the raw files (RawWriter2p.output_dir); the sim tree goes to <dir>/sim_truth.root")
    ap.add_argument("--set", action="append", default=[], metavar="SECTION.KEY=VALUE",
                    help="override a parameter, e.g. physics.em_msc=urban (repeatable)")
    args = ap.parse_args()

    cfg = ec.load_sim(args.config)
    if args.output:
        cfg["pickup"][0]["parameters"]["output"] = args.output
    if args.seed is not None:
        cfg["geant4"]["parameters"]["random_seed"] = args.seed
        for i, entry in enumerate(cfg.get("response", [])):
            if "random_seed" in entry.get("parameters", {}):
                entry["parameters"]["random_seed"] = args.seed * 100 + i + 1
    if args.raw_dir:
        for entry in cfg.get("response", []):
            if entry["module"] == "RawWriter2p":
                entry["parameters"]["output_dir"] = args.raw_dir
                entry["parameters"]["truth_file"] = os.path.join(args.raw_dir, "sim_truth.root")
            if entry["module"] == "HAEmulator" and "cal_output_dir" in entry.get("parameters", {}):
                entry["parameters"]["cal_output_dir"] = args.raw_dir
    for text in args.set:
        apply_override(cfg, text)

    # environment first: Geant4 reads e.g. G4NEUTRONHPDATA when it builds the physics tables
    for key, value in cfg.get("environment", {}).items():
        os.environ[key] = str(value)
        print(f"run_sim.py: {key}={value}")

    import anlnext
    import anlgeant4
    chain = anlnext.AnalysisChain()
    for section in SECTIONS:
        add_module(chain, cfg[section])
    chain.chain(anlgeant4.StandardUserActionAssembly)
    for entry in cfg.get("pickup", []):
        add_module(chain, entry)
    for entry in cfg.get("response", []):
        add_module(chain, entry)
    from anl_run import run_checked
    if not run_checked(chain, args.n_events):
        sys.exit(1)


if __name__ == "__main__":
    main()
