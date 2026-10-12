# Environment for the EricaNext simulation (Geant4 + ANLNext with vectors + ComptonSoft/anlgeant4).
# Usage:  source ~/opt/setup_sim.sh
# (The normal EricaNext data chain keeps using /usr/local ANLNext; don't source this for it.)
source $HOME/opt/geant4-11.4.3/bin/geant4.sh
export ANLNEXT_INSTALL=$HOME/opt/anlnext-g4
export COMPTONSOFT_INSTALL=$HOME/opt/comptonsoft
export LD_LIBRARY_PATH=$COMPTONSOFT_INSTALL/lib:$ANLNEXT_INSTALL/lib:$HOME/opt/geant4-11.4.3/lib64:${LD_LIBRARY_PATH:-}
export PYTHONPATH=$COMPTONSOFT_INSTALL/lib/python:$ANLNEXT_INSTALL/lib/python:${PYTHONPATH:-}
export SMILE2P_SIM_DATA=$HOME/smile2p_sim_data
