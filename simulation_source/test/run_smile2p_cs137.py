"""Cs-137 (662 keV) point source at (0, 0, -1626) mm on the SMILE-2+ geometry
(SMILE_2p_Geo, ground). Optionally writes the geometry to GDML.
Run after `source ~/opt/setup_sim.sh`:  python3 run_smile2p_cs137.py [n_events] [--gdml out.gdml]"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "install", "lib", "python"))
import anlnext
import anlgeant4
import EricaSimPy as sim
sys.path.insert(0, os.path.join(HERE, "..", "..", "config"))
import ericanext_config as ec

SIM_DATA = str(ec.sim_data())
args = sys.argv[1:]
gdml_out = None
if "--gdml" in args:
    i = args.index("--gdml"); gdml_out = args[i + 1]; del args[i:i + 2]
n = int(args[0]) if args else 10000

a = anlnext.AnalysisChain()
a.chain(sim.SMILE2pGeometry)
a.with_parameters({"geometry_dir": os.path.join(SIM_DATA, "SMILE_2p_Geo"),
                   "geometry_type": "ground",
                   "ha_position_file": os.path.join(SIM_DATA, "HA_Parameters_20190403_position_only", "HAs.pos"),
                   "gas_pressure": 2.0})
a.chain(sim.SMILEPhysicsList)
a.chain(anlgeant4.PointSourcePrimaryGen)
a.with_parameters({"particle": "gamma", "spectral_distribution": "mono", "energy_mean": 661.657,
                   "theta_min": 0., "theta_max": 0.25})
a.with_setter(lambda m: m.set_parameter("position", 0., 0., -162.6))   # cm
a.with_setter(lambda m: m.set_parameter("direction", 0., 0., 1.))
a.chain(anlgeant4.Geant4Body)
a.with_parameters({"random_engine": "MTwistEngine", "random_initialization_mode": 1,
                   "random_seed": 1, "output_random_status": False, "verbose": 0})
a.chain(anlgeant4.StandardUserActionAssembly)
a.chain(sim.EdepPickUp);  a.with_parameters({"output": os.path.join(HERE, "smile2p_cs137_edep.root")})
if gdml_out:
    a.chain(anlgeant4.WriteGDML);  a.with_parameters({"file": gdml_out})
a.run(n)
