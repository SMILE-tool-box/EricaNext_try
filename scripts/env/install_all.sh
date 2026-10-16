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
#
# A finished step is marked in ~/opt/.ericanext_install/ and skipped when the script is run again,
# so after a failure just run it again: it continues with the failed step.
#   bash install_all.sh            continue (skip the finished steps)
#   bash install_all.sh --from 3   redo from step 3 (e.g. after changing ANLNext)
# The output is also written to ~/opt/logs/install_<date>.log.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
STAMP=$HOME/opt/.ericanext_install
mkdir -p $HOME/opt/src $HOME/opt/logs $STAMP
if [ "${1:-}" = "--from" ]; then        # redo from this step: forget it and the later ones
  FROM=${2:?"--from needs a step number (1-6)"}
  for n in $(seq $FROM 6); do rm -f $STAMP/step$n.done; done
fi

LOG=$HOME/opt/logs/install_$(date +%Y%m%d_%H%M%S).log
exec > >(tee -a "$LOG") 2>&1
echo "=== install_all.sh $(date)  (log: $LOG)"

# the Geant4 used is recorded at step 1 so that later runs (e.g. --from 3) use the same one
if [ -n "${GEANT4_INSTALL:-}" ]; then echo "$GEANT4_INSTALL" > $STAMP/geant4_install
elif [ -f $STAMP/geant4_install ]; then GEANT4_INSTALL=$(cat $STAMP/geant4_install)
fi

step() {   # step <n> <title> <command...>
  local n=$1 title=$2; shift 2
  if [ -f $STAMP/step$n.done ]; then echo "=== step $n ($title): done before, skipped"; return; fi
  echo "=== step $n ($title)"
  if ! "$@"; then
    echo "!!! step $n ($title) failed. Fix the cause (see $LOG) and run install_all.sh again: it continues from step $n."
    exit 1
  fi
  touch $STAMP/step$n.done
}

geant4() {
  if [ -n "${GEANT4_INSTALL:-}" ]; then
    echo "using the existing Geant4 in $GEANT4_INSTALL"
  else
    bash $HERE/build_geant4.sh
    GEANT4_INSTALL=$HOME/opt/geant4-11.4.3
    echo "$GEANT4_INSTALL" > $STAMP/geant4_install
  fi
  GEANT4_INSTALL=$GEANT4_INSTALL bash $HERE/check_geant4.sh
}
setup_file() {
  sed "s#^export GEANT4_INSTALL=.*#export GEANT4_INSTALL=$GEANT4_INSTALL#" $HERE/setup_ericanext.sh > $HOME/opt/setup_ericanext.sh
}

step 1 "Geant4" geant4
export GEANT4_INSTALL=${GEANT4_INSTALL:-$HOME/opt/geant4-11.4.3}
step 2 "neutron data" bash $HERE/make_g4ndl_ar36fix.sh
step 3 "ANLNext and ComptonSoft" bash $HERE/build_anlnext_comptonsoft.sh
step 4 "environment ~/opt/setup_ericanext.sh" setup_file
step 5 "EricaNext_try" bash $HERE/build_ericanext.sh
step 6 "Jupyter kernel" bash $HERE/make_jupyter_kernel.sh
cat <<MSG
=== done (log: $LOG)
  terminal: source ~/opt/setup_ericanext.sh   (only when you use EricaNext; put it in ~/.bashrc only if you use no other Geant4 / ANLNext)
  Jupyter : choose the kernel "EricaNext"
MSG
