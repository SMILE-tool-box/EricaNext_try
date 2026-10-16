#!/bin/bash
# Install everything EricaNext_try needs under ~/opt, then build EricaNext_try. /usr/local is not touched,
# and nothing outside ~/opt is changed (~/.bashrc is not edited).
#   1. Geant4 11.4.3 (GDML, MT, global-dynamic TLS)              build_geant4.sh
#      or an existing Geant4:  GEANT4_INSTALL=/path/to/geant4 bash install_all.sh   (checked by check_geant4.sh)
#   2. neutron data G4NDL + Ar-36/38 of G4NDL4.5                 make_g4ndl_ar36fix.sh
#   3. ANLNext (Geant4-CLHEP vectors) and ComptonSoft (anlgeant4) build_anlnext_comptonsoft.sh
#   4. ~/opt/setup_ericanext.sh (environment)
#   5. EricaNext_try (data chain and simulation)                  build_ericanext.sh
#   6. Jupyter kernel "EricaNext"                                  make_jupyter_kernel.sh
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p $HOME/opt/src
if [ -n "${GEANT4_INSTALL:-}" ]; then
  echo "=== using the existing Geant4 in $GEANT4_INSTALL"
  bash $HERE/check_geant4.sh
else
  bash $HERE/build_geant4.sh
  export GEANT4_INSTALL=$HOME/opt/geant4-11.4.3
fi
bash $HERE/make_g4ndl_ar36fix.sh
bash $HERE/build_anlnext_comptonsoft.sh
sed "s#^export GEANT4_INSTALL=.*#export GEANT4_INSTALL=$GEANT4_INSTALL#" $HERE/setup_ericanext.sh > $HOME/opt/setup_ericanext.sh
bash $HERE/build_ericanext.sh
bash $HERE/make_jupyter_kernel.sh
cat <<MSG
done.
  terminal: source ~/opt/setup_ericanext.sh   (only when you use EricaNext; put it in ~/.bashrc only if you use no other Geant4 / ANLNext)
  Jupyter : choose the kernel "EricaNext"
MSG
