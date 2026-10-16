#!/bin/bash
# Build EricaNext_try (data chain: source/ -> install/, simulation: simulation_source/ -> simulation_source/install/)
# against ANLNext in ~/opt/anlnext-g4. Run after install_all.sh (or with ~/opt/setup_ericanext.sh sourced).
set -euo pipefail
REPO=$(cd "$(dirname "$0")/../.." && pwd)
J=$(nproc)
source $HOME/opt/setup_ericanext.sh

# --- data chain (EricaPy)
cd $REPO
rm -rf build install && mkdir build install && cd build
cmake ../source -DCMAKE_INSTALL_PREFIX=../install -DANLNEXT_INSTALL=$ANLNEXT_INSTALL -DPython3_EXECUTABLE=/usr/bin/python3
make -j$J && make install

# --- simulation (EricaSimPy)
cd $REPO
rm -rf simulation_source/build simulation_source/install
cmake -S simulation_source -B simulation_source/build -DCMAKE_INSTALL_PREFIX=simulation_source/install
cmake --build simulation_source/build --target install -j$J
