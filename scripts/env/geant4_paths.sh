# Geant4 used by the EricaNext_try scripts (sourced by the other scripts).
#   $GEANT4_INSTALL: an existing Geant4 install (its prefix, with bin/geant4.sh); default ~/opt/geant4-11.4.3 (build_geant4.sh)
# Sets G4_CMAKE_DIR (directory of Geant4Config.cmake), G4_LIBDIR (the Geant4 libraries) and sources geant4.sh.
export GEANT4_INSTALL=${GEANT4_INSTALL:-$HOME/opt/geant4-11.4.3}
G4_CMAKE_DIR=$(dirname "$(ls $GEANT4_INSTALL/lib*/cmake/Geant4*/Geant4Config.cmake 2>/dev/null | head -1)")
G4_LIBDIR=$(cd "$G4_CMAKE_DIR/../.." 2>/dev/null && pwd)
source $GEANT4_INSTALL/bin/geant4.sh
