# Environment for EricaNext_try (data reconstruction and simulation):
# Geant4 + ANLNext built with Geant4-CLHEP vectors + ComptonSoft (anlgeant4), all under ~/opt.
# Usage:  source ~/opt/setup_ericanext.sh   (install_all.sh copies this file to ~/opt; ~/.bashrc sources it)
# ANLNext needs libG4clhep.so even for `import anlnext`, so the data chain needs this environment too.
source $HOME/opt/geant4-11.4.3/bin/geant4.sh
export ANLNEXT_INSTALL=$HOME/opt/anlnext-g4
export COMPTONSOFT_INSTALL=$HOME/opt/comptonsoft
export LD_LIBRARY_PATH=$COMPTONSOFT_INSTALL/lib:$ANLNEXT_INSTALL/lib:$HOME/opt/geant4-11.4.3/lib64:${LD_LIBRARY_PATH:-}
export PYTHONPATH=$COMPTONSOFT_INSTALL/lib/python:$ANLNEXT_INSTALL/lib/python:${PYTHONPATH:-}
export RUBYLIB=$ANLNEXT_INSTALL/lib/ruby:${RUBYLIB:-}
# Neutron data: G4NDL4.7.1 + the Ar-36/38 files of G4NDL4.5 (make_g4ndl_ar36fix.sh);
# plain G4NDL4.7.1 treats Ar-36 with the Ar-37 data (spurious (n,alpha) in the TPC gas).
export G4NEUTRONHPDATA=$HOME/opt/geant4-data-extra/G4NDL4.7.1_Ar36fix
