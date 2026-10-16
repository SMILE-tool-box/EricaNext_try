#!/bin/bash
# Check that the Geant4 in $GEANT4_INSTALL (default ~/opt/geant4-11.4.3) can be used by EricaNext_try:
#   GDML, the global-dynamic TLS model when multithreaded (needed to load Geant4 from Python),
#   the physics data, and the version (tested: 11.4.3).
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
source $HERE/geant4_paths.sh
ok=1
CFG=$GEANT4_INSTALL/bin/geant4-config
[ -x $CFG ] || { echo "NG  no $CFG"; exit 1; }
v=$($CFG --version); echo "    Geant4 $v at $GEANT4_INSTALL (cmake: $G4_CMAKE_DIR, libraries: $G4_LIBDIR)"
[ "$v" = "11.4.3" ] || echo "WARN version $v: EricaNext_try is tested with 11.4.3 (ComptonSoft bbe3486 needs Geant4 11)"
if [ "$($CFG --has-feature gdml)" = yes ]; then echo "OK  GDML"; else echo "NG  GDML is off (rebuild with -DGEANT4_USE_GDML=ON)"; ok=0; fi
if [ "$($CFG --has-feature multithreading)" = yes ]; then
  tls=$(sed -n 's/^ *set(Geant4_TLS_MODEL "\(.*\)")/\1/p' $G4_CMAKE_DIR/Geant4Config.cmake | head -1)
  if [ "$tls" = global-dynamic ]; then echo "OK  multithreaded, TLS model global-dynamic"
  else echo "NG  multithreaded with TLS model '${tls:-default}': Python cannot load it (rebuild with -DGEANT4_BUILD_TLS_MODEL=global-dynamic)"; ok=0; fi
else
  echo "OK  sequential build (no TLS problem)"
fi
missing=$($CFG --check-datasets | grep -v -i "INSTALLED" || true)
if [ -z "$missing" ]; then echo "OK  physics data"; else echo "NG  physics data missing (geant4-config --install-datasets):"; echo "$missing"; ok=0; fi
[ $ok = 1 ] && echo "=> this Geant4 can be used" || { echo "=> this Geant4 cannot be used"; exit 1; }
