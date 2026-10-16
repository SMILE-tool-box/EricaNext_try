#!/bin/bash
# Build ANLNext (with Geant4-CLHEP vectors) and ComptonSoft (anlgeant4) under ~/opt,
# after build_geant4.sh (or with $GEANT4_INSTALL set to an existing Geant4). This one ANLNext is used for both the EricaNext data chain and the simulation.
# /usr/local is not touched.
#   ANLNext source:     $ANLNEXT_SRC, ~/ANLNext (only if at c22eb3e) or ~/opt/src/ANLNext (github.com/odakahirokazu/ANLNext, c22eb3e)
#   ComptonSoft source: ~/opt/src/ComptonSoft (github.com/odakahirokazu/ComptonSoft, bbe3486)
set -euo pipefail
OPT=$HOME/opt; J=$(nproc)
source $(cd "$(dirname "$0")" && pwd)/geant4_paths.sh   # $GEANT4_INSTALL, G4_CMAKE_DIR, G4_LIBDIR

# --- ANLNext with ANLNEXT_USE_G4VECTOR (anlgeant4 requires vector parameters)
# The tested versions. ComptonSoft bbe3486 does not build with newer ANLNext (e.g. 9898a47: BasicModule::help_duplication
# returns a unique_ptr that GCC 11 / C++17 cannot move), so the ANLNext commit matters.
ANLNEXT_COMMIT=c22eb3e
COMPTONSOFT_COMMIT=bbe3486
# clone <url> <dir> <commit>: into <dir>.part first, so an interrupted clone is never used
clone_at() {
  rm -rf "$2.part"
  git clone "$1" "$2.part" && git -C "$2.part" checkout -q "$3" && mv "$2.part" "$2"
}
at_commit() {   # at_commit <dir> <commit>: the git checkout in <dir> is at <commit>
  [ "$(git -C "$1" rev-parse HEAD 2>/dev/null)" = "$(git -C "$1" rev-parse "$2^{commit}" 2>/dev/null)" ]
}
# source: $ANLNEXT_SRC; else ~/ANLNext if it is a checkout of the tested commit (it is not changed otherwise);
# else ~/opt/src/ANLNext at the tested commit (cloned if missing)
if [ -n "${ANLNEXT_SRC:-}" ]; then
  at_commit $ANLNEXT_SRC $ANLNEXT_COMMIT || echo "WARNING: $ANLNEXT_SRC is not ANLNext $ANLNEXT_COMMIT (tested); ComptonSoft may not build"
elif [ -d $HOME/ANLNext/source ] && at_commit $HOME/ANLNext $ANLNEXT_COMMIT; then
  ANLNEXT_SRC=$HOME/ANLNext
else
  [ -d $HOME/ANLNext/source ] && echo "~/ANLNext is not ANLNext $ANLNEXT_COMMIT: it is left as it is, and $OPT/src/ANLNext is used"
  ANLNEXT_SRC=$OPT/src/ANLNext
  [ -d $ANLNEXT_SRC/source ] || clone_at https://github.com/odakahirokazu/ANLNext.git $ANLNEXT_SRC $ANLNEXT_COMMIT
  at_commit $ANLNEXT_SRC $ANLNEXT_COMMIT || git -C $ANLNEXT_SRC checkout -q $ANLNEXT_COMMIT
fi
echo "ANLNext source: $ANLNEXT_SRC ($(git -C $ANLNEXT_SRC log --oneline -1 2>/dev/null))"
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
[ -f ComptonSoft/anlgeant4/CMakeLists.txt ] || { rm -rf ComptonSoft; clone_at https://github.com/odakahirokazu/ComptonSoft.git $OPT/src/ComptonSoft $COMPTONSOFT_COMMIT; }
at_commit $OPT/src/ComptonSoft $COMPTONSOFT_COMMIT || echo "WARNING: $OPT/src/ComptonSoft is not ComptonSoft $COMPTONSOFT_COMMIT (tested)"
echo "ComptonSoft source: $OPT/src/ComptonSoft ($(git -C $OPT/src/ComptonSoft log --oneline -1 2>/dev/null))"
cd ComptonSoft && rm -rf build && mkdir build && cd build
export ANLNEXT_INSTALL=$OPT/anlnext-g4
export LIBRARY_PATH=$OPT/anlnext-g4/lib:$G4_LIBDIR
cmake .. -DCMAKE_INSTALL_PREFIX=$OPT/comptonsoft -DCS_USE_GDML=ON -DCS_USE_VIS_QT=OFF \
  -DCS_USE_RUBY=OFF -DCS_USE_PYTHON=ON -DGeant4_DIR=$G4_CMAKE_DIR \
  -DPython3_EXECUTABLE=/usr/bin/python3 \
  -DCMAKE_INSTALL_RPATH="$OPT/comptonsoft/lib;$OPT/anlnext-g4/lib;$G4_LIBDIR"
make -j$J && make install
