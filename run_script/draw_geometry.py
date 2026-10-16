#!/usr/bin/env python3
"""Draw a GDML geometry to PNG images without a display (Geant4 TSG_OFFSCREEN, EricaSimPy GeometryImage).

    python3 run_sim.py smile2p_flight_parma_thermal_neutron_d10 -n 1 -o /tmp/edep.root --gdml smile2p_flight.gdml
    python3 draw_geometry.py smile2p_flight.gdml -o images/         # images/smile2p_flight_<view>.png

Views (the detector coordinates: z is downward, so the up vector is -z):
    outside   the whole geometry (gondola, pressure vessel, ballast)
    detector  without the gondola, the pressure vessel, the anti and the support structures:
              TPC (drift volume, drift cage, GEM, uPIC) and the HA scintillators with their PMTs
    top       seen from above (-z) without the TPC inside: the layout of the HA modules (6 bottom, 12 side)
Run after `source ~/opt/setup_ericanext.sh` (or with the Jupyter kernel EricaNext).
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, "..")
sys.path.insert(0, os.path.join(REPO, "simulation_source", "install", "lib", "python"))
sys.path.insert(0, HERE)

# colours by a part of the logical-volume name: "<part> r g b [alpha]" (later entries win)
COLOURS = [
    "bCALET 0.80 0.80 0.83", "leg 0.80 0.80 0.83", "flange_support 0.75 0.75 0.78", "ceiling 0.80 0.80 0.83",
    "ballast_hanger 0.55 0.55 0.60", "blst 0.40 0.40 0.45",
    "TPC_side 0.78 0.80 0.84", "TPC_top 0.78 0.80 0.84", "TPC_bottom 0.78 0.80 0.84", "port 0.70 0.70 0.75",
    "anti 1.00 0.65 0.20", "base 0.75 0.70 0.60", "batter 0.50 0.50 0.50", "board 0.85 0.45 0.40",
    "TPC_detectable 0.55 0.75 1.00 0.35", "H8500 0.35 0.35 0.40", "driftcage 0.95 0.85 0.55", "GEM 0.90 0.75 0.40", "uPIC 0.90 0.60 0.30",
    "scinti 0.20 0.75 0.30", "esr 0.20 0.75 0.30", "tefron 0.20 0.75 0.30", "black_tape 0.15 0.60 0.25",
]
GONDOLA = ["bCALET", "leg", "ballast_hanger", "flange_support", "ceiling", "blst_0", "blst_1", "blst_2", "blst_3"]
VESSEL = ["TPC_side", "TPC_top", "TPC_bottom_high", "TPC_gas_bottom", "port1", "port2"]
OUTER_PARTS = ["anti", "base", "battery1", "battery2", "batterycase_1", "batterycase_2", "boards", "boards1", "boards2",
               "boards3", "tana", "tana_top"]
# support structures inside the TPC gas volume (hidden in the "detector" and "top" views)
STRUCTURE = (["igeta", "pole", "shield", "insulate_bottom", "mesh_top", "mesh_bottom",
              "drift_mesh_frame", "gem_frame", "wire", "Lji_array", "chukei_kiban", "psa_board", "GEM_base", "uPIC_base"]
             + [f"insulate{i}" for i in range(1, 5)] + [f"tanoji_bottom{i}" for i in range(1, 7)]
             + [f"tanoji_side{i}" for i in range(1, 13)])

TPC_INNER = ["uPIC_Anode", "uPIC_Cathode", "uPIC_polyimide", "GEM_up", "GEM_down", "TPC_detectable"] + \
    [f"driftcage_side{i}" for i in range(1, 5)]

VIEWS = {
    # name: (volumes hidden, viewpoint vector, up vector, zoom)
    "outside": ([], "1 -0.8 -0.6", "0 0 -1", 1.0),
    "detector": (GONDOLA + VESSEL + OUTER_PARTS + STRUCTURE, "1 -0.8 -0.6", "0 0 -1", 1.1),
    "top": (GONDOLA + VESSEL + OUTER_PARTS + STRUCTURE + TPC_INNER, "0 0 -1", "0 -1 0", 1.1),
}


def view_commands(name, png, first):
    hidden, viewpoint, up, zoom = VIEWS[name]
    cmds = []
    if first:
        cmds += ["/vis/open TSG_OFFSCREEN 1400x1200", "/vis/viewer/set/autoRefresh false", "/vis/drawVolume",
                 "/vis/viewer/set/style surface", "/vis/viewer/set/background white",
                 "/vis/geometry/set/visibility World 0 false", "/vis/geometry/set/visibility Air 0 false",
                 "/vis/geometry/set/visibility TPC_gas 0 false"]
    shown = set(GONDOLA + VESSEL + OUTER_PARTS + STRUCTURE + TPC_INNER) - set(hidden)
    cmds += [f"/vis/geometry/set/visibility {v} 0 false" for v in hidden]
    cmds += [f"/vis/geometry/set/visibility {v} 0 true" for v in sorted(shown)]
    cmds += [f"/vis/viewer/set/upVector {up}", f"/vis/viewer/set/viewpointVector {viewpoint}",
             f"/vis/viewer/set/lightsVector {viewpoint}",
             "/vis/viewer/zoomTo %g" % zoom,
             f"/vis/tsg/offscreen/set/file {png}", "/vis/viewer/rebuild"]
    return cmds


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("gdml", help="GDML file (e.g. from run_sim.py --gdml)")
    ap.add_argument("-o", "--outdir", default=".", help="directory of the images (<gdml name>_<view>.png)")
    ap.add_argument("--views", default=",".join(VIEWS), help=f"comma-separated views ({', '.join(VIEWS)})")
    args = ap.parse_args()

    os.makedirs(args.outdir, exist_ok=True)
    stem = os.path.splitext(os.path.basename(args.gdml))[0]
    cmds = []
    for i, name in enumerate(args.views.split(",")):
        png = os.path.abspath(os.path.join(args.outdir, f"{stem}_{name}.png"))
        cmds += view_commands(name, png, i == 0)

    import anlnext
    import anlgeant4
    import EricaSimPy as sim
    from anl_run import run_checked
    chain = anlnext.AnalysisChain()
    chain.chain(anlgeant4.ReadGDML)
    chain.with_parameters({"file": os.path.abspath(args.gdml), "validate": False})
    chain.chain(sim.SMILEPhysicsList)             # Geant4Body needs a physics list and a primary generator
    chain.chain(anlgeant4.PointSourcePrimaryGen)
    chain.chain(anlgeant4.Geant4Body)
    chain.with_parameters({"output_random_status": False})   # no RandomSeed_*.dat
    chain.chain(anlgeant4.StandardUserActionAssembly)
    chain.chain(sim.GeometryImage)
    chain.with_parameters({"commands": cmds, "colours": COLOURS})
    if not run_checked(chain, 0):
        sys.exit(1)


if __name__ == "__main__":
    main()
