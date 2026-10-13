#!/usr/bin/env python3
"""Run the EricaNext simulation (Geant4 via anlgeant4) with a configuration from
config/simulation/*.json. Run after `source ~/opt/setup_sim.sh`.

    python3 run_sim.py smile2p_flight_parma_thermal_neutron_d10 -n 1000000 -o out.root --seed 1
    python3 run_sim.py my_setting.json -n 1000 -o test.root --set physics.em_msc=urban

The config has the sections environment, detector, physics, source, geant4 and pickup
(see config/simulation/README.md). The chain is
    detector -> physics -> source -> geant4 (Geant4Body) -> StandardUserActionAssembly -> pickup...
Modules are looked up in EricaSimPy, then anlgeant4.
"""
import argparse
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, "..")
sys.path.insert(0, os.path.join(REPO, "simulation", "install", "lib", "python"))
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
    elif section == "pickup":
        cfg["pickup"][0]["parameters"][key] = value
    else:
        cfg[section]["parameters"][key] = value


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("config", help=f"config name or .json path ({', '.join(ec.available_sim())})")
    ap.add_argument("-n", "--n-events", type=int, required=True, help="number of primary events")
    ap.add_argument("-o", "--output", help="output ROOT file of the first pickup module")
    ap.add_argument("--seed", type=int, help="random seed (geant4.random_seed)")
    ap.add_argument("--set", action="append", default=[], metavar="SECTION.KEY=VALUE",
                    help="override a parameter, e.g. physics.em_msc=urban (repeatable)")
    args = ap.parse_args()

    cfg = ec.load_sim(args.config)
    if args.output:
        cfg["pickup"][0]["parameters"]["output"] = args.output
    if args.seed is not None:
        cfg["geant4"]["parameters"]["random_seed"] = args.seed
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
    chain.run(args.n_events)


if __name__ == "__main__":
    main()
