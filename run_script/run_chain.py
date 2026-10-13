#!/usr/bin/env python3
"""Run the EricaNext chain with a configuration from config/*.json.

    python3 run_chain.py smile2p_20190701_Per02 -o per02_out.root
    python3 run_chain.py smile2p_20190701_Per02 -o raw.root --raw -n 10000

--raw runs ReadData -> RawTreeWriter (event-synchronized raw data tree)
instead of the reconstruction chain.
--set Module.param=value overrides a parameter of the config (repeatable), e.g.
    python3 run_chain.py smile2p_20190701_Per02 -o raw.root --raw --set RawTreeWriter.save_threshold=10
"""
import argparse
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, "..")
# use this repository's build, not an older EricaPy in /usr/local/lib/python
sys.path.insert(0, os.path.join(REPO, "install", "lib", "python"))
sys.path.insert(0, os.path.join(REPO, "config"))
import anlnext
import EricaPy as th
import ericanext_config as ec


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("config", help=f"config name or .json path ({', '.join(ec.available())})")
    ap.add_argument("-o", "--output", required=True, help="output ROOT file")
    ap.add_argument("-n", "--n-events", type=int, default=-1, help="number of events (-1 = all)")
    ap.add_argument("--raw", action="store_true", help="ReadData -> RawTreeWriter only")
    ap.add_argument("--compression", type=int, default=-1,
                    help="ROOT compression setting (505 = zstd, recommended with --raw)")
    ap.add_argument("--set", action="append", default=[], metavar="MODULE.PARAM=VALUE",
                    help="override a module parameter (value parsed as JSON when possible), repeatable")
    args = ap.parse_args()

    cfg = ec.load(args.config)
    params = cfg["parameters"]
    for text in args.set:
        path, value = text.split("=", 1)
        module, key = path.split(".", 1)
        try:
            value = json.loads(value)
        except json.JSONDecodeError:
            pass
        params.setdefault(module, {})[key] = value
    modules = ["ReadData", "RawTreeWriter"] if args.raw else cfg["chain"]

    a = anlnext.AnalysisChain()
    a.chain(th.SaveFile)
    a.with_parameters({"filename": args.output, "compression": args.compression})
    for m in modules:
        a.chain(getattr(th, m))
        if params.get(m):
            a.with_parameters(params[m])
    a.run(args.n_events)


if __name__ == "__main__":
    main()
