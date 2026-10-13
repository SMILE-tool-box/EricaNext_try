#!/bin/bash
# G4NDL4.7.1 has no Ar-36 / Ar-38 files (removed in 4.7.1), and Geant4 then uses the Ar-37 data
# for Ar-36: natural argon gets a spurious (n,alpha) of ~6.7 b for thermal neutrons
# (docs/simulation_port_log.md). This makes G4NDL4.7.1_Ar36fix = G4NDL4.7.1 (hard links, no extra
# space) + the Ar-36 / Ar-38 files of G4NDL4.5 (ENDF/B-VII.1, the data of the SMILE_tool_box runs).
# setup_sim.sh points G4NEUTRONHPDATA to it. Never edit files in it in place (hard links).
set -euo pipefail
OPT=$HOME/opt; GV=11.4.3
NDL=$OPT/geant4-data-$GV/G4NDL4.7.1
EXTRA=$OPT/geant4-data-extra
mkdir -p $EXTRA && cd $EXTRA
if [ ! -d G4NDL4.5 ]; then
  curl -sSfL -o G4NDL.4.5.tar.gz https://cern.ch/geant4-data/datasets/G4NDL.4.5.tar.gz
  tar xzf G4NDL.4.5.tar.gz && rm G4NDL.4.5.tar.gz
fi
rm -rf G4NDL4.7.1_Ar36fix
cp -al $NDL G4NDL4.7.1_Ar36fix
cd G4NDL4.5
find . \( -name "18_36_Argon*" -o -name "18_38_Argon*" \) | while read -r f; do
  t=../G4NDL4.7.1_Ar36fix/$f
  [ -e "$t" ] && { echo "unexpected: $t exists"; exit 1; }
  mkdir -p "$(dirname "$t")" && cp -p "$f" "$t"
done
echo "added $(find ../G4NDL4.7.1_Ar36fix -name '18_3[68]_Argon*' | wc -l) Ar-36/38 files to $EXTRA/G4NDL4.7.1_Ar36fix"
