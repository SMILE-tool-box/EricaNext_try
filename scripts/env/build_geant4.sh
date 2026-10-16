#!/bin/bash
# Build Geant4 (with GDML and data) under ~/opt, without touching /usr/local.
# xerces-c and expat come from the system (dnf: xerces-c-devel, expat-devel).
set -euo pipefail
OPT=$HOME/opt; SRC=$OPT/src; J=$(nproc)
mkdir -p $SRC
GV=11.4.3
cd $SRC
[ -f geant4-v$GV.tar.gz ] || curl -sSL -o geant4-v$GV.tar.gz https://github.com/Geant4/geant4/archive/refs/tags/v$GV.tar.gz
rm -rf geant4-$GV $OPT/geant4-$GV; tar xzf geant4-v$GV.tar.gz   # physics data kept in $OPT/geant4-data-$GV

echo "=== geant4 $GV"
# physics data (~2 GB): downloaded on the first build, reused when $OPT/geant4-data-$GV already exists
if [ -d $OPT/geant4-data-$GV ]; then INSTALL_DATA=OFF; else INSTALL_DATA=ON; fi
cmake -S geant4-$GV -B geant4-$GV/build -DCMAKE_INSTALL_PREFIX=$OPT/geant4-$GV -DCMAKE_BUILD_TYPE=Release \
  -DGEANT4_USE_GDML=ON -DGEANT4_INSTALL_DATA=$INSTALL_DATA -DGEANT4_INSTALL_DATADIR=$OPT/geant4-data-$GV -DGEANT4_USE_SYSTEM_EXPAT=ON -DGEANT4_BUILD_MULTITHREADED=ON -DGEANT4_BUILD_TLS_MODEL=global-dynamic
cmake --build geant4-$GV/build -j$J
cmake --install geant4-$GV/build
echo "=== done"
$OPT/geant4-$GV/bin/geant4-config --version --has-feature gdml
