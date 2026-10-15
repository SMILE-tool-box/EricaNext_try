"""Test of the Gd neutron capture model (SMILEPhysicsList gd_physics): thermal neutrons
(25 meV) start at the centre of a 1 m GSO cube, which absorbs nearly all of the capture
gammas. With gd_physics the summed energy peaks at the ggarnet totals (Gd-157 7.87 MeV,
Gd-155 8.46 MeV); without it at the Geant4 ENDF / Q-values (7.94, 8.54 MeV).
Run after `source ~/opt/setup_sim.sh`:
    python3 run_gd_capture_test.py <on|off> [n_events] [output.root]"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "install", "lib", "python"))
import anlnext
import anlgeant4
import EricaSimPy as sim
sys.path.insert(0, os.path.join(HERE, "..", "..", "config"))
import ericanext_config as ec

DATA = str(ec.sim_data() / "Gd_capture")
gd = sys.argv[1] == "on"
n = int(sys.argv[2]) if len(sys.argv) > 2 else 2000
out = sys.argv[3] if len(sys.argv) > 3 else os.path.join(HERE, f"gd_capture_test_{sys.argv[1]}.root")

a = anlnext.AnalysisChain()
a.chain(anlgeant4.ReadGDML)
a.with_parameters({"file": os.path.join(HERE, "gd_capture_test.gdml"), "validate": False})
a.chain(sim.SMILEPhysicsList)
a.with_parameters({"gd_physics": gd,
                   "gd155_table": os.path.join(DATA, "Gd155.dat"),
                   "gd157_table": os.path.join(DATA, "Gd157_org.dat")})
a.chain(anlgeant4.PointSourcePrimaryGen)
a.with_parameters({"particle": "neutron", "spectral_distribution": "mono", "energy_mean": 25.3e-6,  # keV
                   "theta_min": 0., "theta_max": 3.14159265})
a.with_setter(lambda m: m.set_parameter("position", 0., 0., 0.))   # cm
a.with_setter(lambda m: m.set_parameter("direction", 0., 0., 1.))
a.chain(anlgeant4.Geant4Body)
a.with_parameters({"random_engine": "MTwistEngine", "random_initialization_mode": 1,
                   "random_seed": 1, "output_random_status": False, "verbose": 0})
a.chain(anlgeant4.StandardUserActionAssembly)
a.chain(sim.EdepPickUp)
a.with_parameters({"output": out})
a.run(n)
