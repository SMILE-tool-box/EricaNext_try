"""Smoke test of the simulation stack: Cs-137 (662 keV) point source at
(0, 0, -1626) mm on the simple test geometry. Run after `source ~/opt/setup_sim.sh`."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "install", "lib", "python"))
import anlnext
import anlgeant4
import EricaSimPy as sim

n = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
a = anlnext.AnalysisChain()
a.chain(anlgeant4.ReadGDML)
a.with_parameters({"file": os.path.join(HERE, "smoke_test.gdml"),
                   "validate": False})   # validation would fetch the GDML schema over the network
a.chain(sim.SMILEPhysicsList)
a.chain(anlgeant4.PointSourcePrimaryGen)
a.with_parameters({"particle": "gamma", "spectral_distribution": "mono", "energy_mean": 661.657,
                   "theta_min": 0., "theta_max": 0.25})
# 3-vector parameters: the Python chain has no list -> 3-vector conversion, so set them directly
a.with_setter(lambda m: m.set_parameter("position", 0., 0., -162.6))   # cm (anlgeant4 length unit)
a.with_setter(lambda m: m.set_parameter("direction", 0., 0., 1.))
a.chain(anlgeant4.Geant4Body)
a.with_parameters({"random_engine": "MTwistEngine", "random_initialization_mode": 1,
                   "random_seed": 1, "output_random_status": False, "verbose": 0})
a.chain(anlgeant4.StandardUserActionAssembly)
a.chain(sim.EdepPickUp);  a.with_parameters({"output": os.path.join(HERE, "smoke_test_edep.root")})
a.run(n)
