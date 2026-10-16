#!/bin/bash
# Build ANLNext (with Geant4-CLHEP vectors) and ComptonSoft (anlgeant4) under ~/opt,
# after build_geant4.sh. This one ANLNext is used for both the EricaNext data chain and the simulation.
# /usr/local is not touched.
#   ANLNext source:     ~/ANLNext            (github.com/odakahirokazu/ANLNext, c22eb3e)
#   ComptonSoft source: ~/opt/src/ComptonSoft (github.com/odakahirokazu/ComptonSoft, bbe3486)
set -euo pipefail
OPT=$HOME/opt; GV=11.4.3; J=$(nproc)
source $OPT/geant4-$GV/bin/geant4.sh

# --- ANLNext with ANLNEXT_USE_G4VECTOR (anlgeant4 requires vector parameters)
[ -d $HOME/ANLNext/source ] || { git clone https://github.com/odakahirokazu/ANLNext.git $HOME/ANLNext; git -C $HOME/ANLNext checkout c22eb3e; }
cd $HOME/ANLNext
rm -rf build-g4 && mkdir build-g4 && cd build-g4
export LIBRARY_PATH=$OPT/geant4-$GV/lib64:${LIBRARY_PATH:-}   # ANLNext links G4clhep without a link dir
cmake .. -DCMAKE_INSTALL_PREFIX=$OPT/anlnext-g4 -DANLNEXT_USE_G4VECTOR=ON \
  -DGeant4_DIR=$OPT/geant4-$GV/lib64/cmake/Geant4 -DPython3_EXECUTABLE=/usr/bin/python3 \
  -DRUBY_CONFIG_FILE_DIR=/usr/include \
  -DCMAKE_INSTALL_RPATH="$OPT/anlnext-g4/lib;$OPT/geant4-$GV/lib64"
make -j$J && make install

# --- ComptonSoft: GDML + Python, no Qt / Ruby (other optional libraries are OFF by default).
#     Do not pass CS_ALL_OFF: it would switch GDML off again.
cd $OPT/src
[ -d ComptonSoft ] || { git clone https://github.com/odakahirokazu/ComptonSoft.git; git -C ComptonSoft checkout bbe3486; }
cd ComptonSoft && rm -rf build && mkdir build && cd build
export ANLNEXT_INSTALL=$OPT/anlnext-g4
export LIBRARY_PATH=$OPT/anlnext-g4/lib:$OPT/geant4-$GV/lib64
cmake .. -DCMAKE_INSTALL_PREFIX=$OPT/comptonsoft -DCS_USE_GDML=ON -DCS_USE_VIS_QT=OFF \
  -DCS_USE_RUBY=OFF -DCS_USE_PYTHON=ON -DGeant4_DIR=$OPT/geant4-$GV/lib64/cmake/Geant4 \
  -DPython3_EXECUTABLE=/usr/bin/python3 \
  -DCMAKE_INSTALL_RPATH="$OPT/comptonsoft/lib;$OPT/anlnext-g4/lib;$OPT/geant4-$GV/lib64"
make -j$J && make install
