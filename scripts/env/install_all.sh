#!/bin/bash
# Install everything EricaNext_try needs under ~/opt, then build EricaNext_try. /usr/local is not touched.
#   1. Geant4 11.4.3 (GDML, MT, global-dynamic TLS)            build_geant4.sh
#   2. neutron data G4NDL4.7.1 + Ar-36/38 of G4NDL4.5           make_g4ndl_ar36fix.sh
#   3. ANLNext (Geant4-CLHEP vectors) and ComptonSoft (anlgeant4) build_anlnext_comptonsoft.sh
#   4. ~/opt/setup_ericanext.sh (environment)
#   5. EricaNext_try (data chain and simulation)                build_ericanext.sh
# Then add `source ~/opt/setup_ericanext.sh` to ~/.bashrc.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p $HOME/opt/src
bash $HERE/build_geant4.sh
bash $HERE/make_g4ndl_ar36fix.sh
bash $HERE/build_anlnext_comptonsoft.sh
cp $HERE/setup_ericanext.sh $HOME/opt/setup_ericanext.sh
bash $HERE/build_ericanext.sh
echo "done. Add to ~/.bashrc:  source ~/opt/setup_ericanext.sh"
