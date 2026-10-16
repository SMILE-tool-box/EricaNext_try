# Environment for EricaNext_try (data reconstruction and simulation):
# Geant4 + ANLNext built with Geant4-CLHEP vectors + ComptonSoft (anlgeant4).
# Usage:  source ~/opt/setup_ericanext.sh   (install_all.sh copies this file to ~/opt with the Geant4 it used)
#   - source it only when you use EricaNext (or from ~/.bashrc if you use no other Geant4 / ANLNext):
#     it puts this Geant4 and ANLNext first in PATH, LD_LIBRARY_PATH and PYTHONPATH.
#   - Jupyter: choose the kernel "EricaNext" (make_jupyter_kernel.sh) instead.
# ANLNext needs libG4clhep.so even for `import anlnext`, so the data chain needs this environment too.
# The neutron data (G4NEUTRONHPDATA) is set by the simulation config files (environment section), not here.
export GEANT4_INSTALL=$HOME/opt/geant4-11.4.3
export ANLNEXT_INSTALL=$HOME/opt/anlnext-g4
export COMPTONSOFT_INSTALL=$HOME/opt/comptonsoft
source $GEANT4_INSTALL/bin/geant4.sh
_g4lib=$(dirname "$(dirname "$(dirname "$(ls $GEANT4_INSTALL/lib*/cmake/Geant4*/Geant4Config.cmake | head -1)")")")
export LD_LIBRARY_PATH=$COMPTONSOFT_INSTALL/lib:$ANLNEXT_INSTALL/lib:$_g4lib:${LD_LIBRARY_PATH:-}
export PYTHONPATH=$COMPTONSOFT_INSTALL/lib/python:$ANLNEXT_INSTALL/lib/python:${PYTHONPATH:-}
export RUBYLIB=$ANLNEXT_INSTALL/lib/ruby:${RUBYLIB:-}
unset _g4lib
