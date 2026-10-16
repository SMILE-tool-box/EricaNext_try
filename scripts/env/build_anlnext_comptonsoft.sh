#!/bin/bash
# Build ANLNext (with Geant4-CLHEP vectors) and ComptonSoft (anlgeant4) under ~/opt,
# after build_geant4.sh (or with $GEANT4_INSTALL set to an existing Geant4). This one ANLNext is used for both the EricaNext data chain and the simulation.
# /usr/local is not touched.
#   ANLNext source:     $ANLNEXT_SRC, ~/ANLNext or ~/opt/src/ANLNext (github.com/odakahirokazu/ANLNext, c22eb3e)
#   ComptonSoft source: ~/opt/src/ComptonSoft (github.com/odakahirokazu/ComptonSoft, bbe3486)
set -euo pipefail
OPT=$HOME/opt; J=$(nproc)
source $(cd "$(dirname "$0")" && pwd)/geant4_paths.sh   # $GEANT4_INSTALL, G4_CMAKE_DIR, G4_LIBDIR

# --- ANLNext with ANLNEXT_USE_G4VECTOR (anlgeant4 requires vector parameters)
# source: $ANLNEXT_SRC, else ~/ANLNext if it holds the ANLNext source, else cloned to ~/opt/src/ANLNext
if [ -z "${ANLNEXT_SRC:-}" ]; then
  if [ -d $HOME/ANLNext/source ]; then ANLNEXT_SRC=$HOME/ANLNext; else ANLNEXT_SRC=$OPT/src/ANLNext; fi
fi
# clone <url> <dir> <commit>: into <dir>.part first, so an interrupted clone is never used
clone_at() {
  rm -rf "$2.part"
  git clone "$1" "$2.part" && git -C "$2.part" checkout -q "$3" && mv "$2.part" "$2"
}
[ -d $ANLNEXT_SRC/source ] || clone_at https://github.com/odakahirokazu/ANLNext.git $ANLNEXT_SRC c22eb3e
cd $ANLNEXT_SRC
rm -rf build-g4 && mkdir build-g4 && cd build-g4
export LIBRARY_PATH=$G4_LIBDIR:${LIBRARY_PATH:-}   # ANLNext links G4clhep without a link dir
cmake .. -DCMAKE_INSTALL_PREFIX=$OPT/anlnext-g4 -DANLNEXT_USE_G4VECTOR=ON \
  -DGeant4_DIR=$G4_CMAKE_DIR -DPython3_EXECUTABLE=/usr/bin/python3 \
  -DRUBY_CONFIG_FILE_DIR=/usr/include \
  -DCMAKE_INSTALL_RPATH="$OPT/anlnext-g4/lib;$G4_LIBDIR"
make -j$J && make install

# --- ComptonSoft: installed only for anlgeant4 (Geant4 as ANLNext modules: Geant4Body, VANLGeometry, BasicPrimaryGen, ...),
#     which exists only inside the ComptonSoft repository; the ComptonSoft detector simulation itself is not used.
#     GDML + Python, no Qt / Ruby (other optional libraries are OFF by default).
#     Do not pass CS_ALL_OFF: it would switch GDML off again.
cd $OPT/src
[ -f ComptonSoft/anlgeant4/CMakeLists.txt ] || { rm -rf ComptonSoft; clone_at https://github.com/odakahirokazu/ComptonSoft.git $OPT/src/ComptonSoft bbe3486; }
cd ComptonSoft && rm -rf build && mkdir build && cd build
export ANLNEXT_INSTALL=$OPT/anlnext-g4
export LIBRARY_PATH=$OPT/anlnext-g4/lib:$G4_LIBDIR
cmake .. -DCMAKE_INSTALL_PREFIX=$OPT/comptonsoft -DCS_USE_GDML=ON -DCS_USE_VIS_QT=OFF \
  -DCS_USE_RUBY=OFF -DCS_USE_PYTHON=ON -DGeant4_DIR=$G4_CMAKE_DIR \
  -DPython3_EXECUTABLE=/usr/bin/python3 \
  -DCMAKE_INSTALL_RPATH="$OPT/comptonsoft/lib;$OPT/anlnext-g4/lib;$G4_LIBDIR"
make -j$J && make install
