"""Cosmic-ray background on the SMILE-2+ flight geometry, as the reference run
hikaru3_analysis2/BGsim/thermal_neutron_g10/simulation (sim.mac: SourceType CR,
GeometryType flight, GdPhysics 1, ParaSrcSize 3000, ParaSrcDistance 3000, HA_OFF.dat).
Run after `source ~/opt/setup_ericanext.sh`:
    python3 run_smile2p_parma_cr.py <flux.root> [n_events] [--particle neutron] [--seed 1] [-o out.root]"""
import argparse, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "install", "lib", "python"))
import anlnext
import anlgeant4
import EricaSimPy as sim
sys.path.insert(0, os.path.join(HERE, "..", "..", "config"))
import ericanext_config as ec

SIM_DATA = str(ec.sim_data())
p = argparse.ArgumentParser()
p.add_argument("flux")
p.add_argument("n", nargs="?", type=int, default=10000)
p.add_argument("--particle", default="neutron")
p.add_argument("--seed", type=int, default=1)
p.add_argument("--em-msc", default="gs", help="gs (Geant4 11) or urban (reference e+- msc)")
p.add_argument("-o", "--output", default=os.path.join(HERE, "smile2p_parma_cr_edep.root"))
args = p.parse_args()

a = anlnext.AnalysisChain()
a.chain(sim.SMILE2pGeometry)
a.with_parameters({"geometry_dir": os.path.join(SIM_DATA, "SMILE_2p_Geo"),
                   "geometry_type": "flight",
                   "ha_position_file": os.path.join(SIM_DATA, "HA_Parameters_20190403_position_only", "HAs.pos"),
                   "gas_pressure": 2.0})
a.chain(sim.SMILEPhysicsList)
a.with_parameters({"gd_physics": True, "em_msc": args.em_msc,
                   "gd155_table": os.path.join(SIM_DATA, "Gd_capture", "Gd155.dat"),
                   "gd157_table": os.path.join(SIM_DATA, "Gd_capture", "Gd157_org.dat")})
a.chain(sim.ParmaCRPrimaryGen)
a.with_parameters({"particle": args.particle, "flux_file": args.flux,
                   "plane_size": 300.0, "plane_distance": 300.0})   # cm (ParaSrcSize / ParaSrcDistance 3000 mm)
a.chain(anlgeant4.Geant4Body)
a.with_parameters({"random_engine": "MTwistEngine", "random_initialization_mode": 1,
                   "random_seed": args.seed, "output_random_status": False, "verbose": 0})
a.chain(anlgeant4.StandardUserActionAssembly)
a.chain(sim.EdepPickUp)
a.with_parameters({"output": args.output, "ha_off_file": os.path.join(SIM_DATA, "HA_OFF.dat")})
a.run(args.n)
