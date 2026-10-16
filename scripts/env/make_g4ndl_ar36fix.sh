#!/bin/bash
# G4NDL4.7.1 has no Ar-36 / Ar-38 files (removed in 4.7.1), and Geant4 then uses the Ar-37 data
# for Ar-36: natural argon gets a spurious (n,alpha) of ~6.7 b for thermal neutrons
# (docs/simulation_port_log.md). This makes G4NDL4.7.1_Ar36fix = G4NDL4.7.1 (hard links, no extra
# space) + the Ar-36 / Ar-38 files of G4NDL4.5 (ENDF/B-VII.1, the data of the SMILE_tool_box runs).
# setup_ericanext.sh points G4NEUTRONHPDATA to it. Never edit files in it in place (hard links).
set -euo pipefail
OPT=$HOME/opt
source $(cd "$(dirname "$0")" && pwd)/geant4_paths.sh
# the neutron data of this Geant4 (G4NDL4.7.1 for Geant4 11.4.3)
NDL=$(geant4-config --datasets | awk '$2 == "G4NEUTRONHPDATA" {print $3}')
EXTRA=$OPT/geant4-data-extra
mkdir -p $EXTRA && cd $EXTRA
if [ ! -d G4NDL4.5 ]; then
  curl -sSfL -o G4NDL.4.5.tar.gz https://cern.ch/geant4-data/datasets/G4NDL.4.5.tar.gz
  tar xzf G4NDL.4.5.tar.gz && rm G4NDL.4.5.tar.gz
fi
OUT=$(basename $NDL)_Ar36fix     # G4NDL4.7.1_Ar36fix for Geant4 11.4.3
rm -rf $OUT
cp -al $NDL $OUT 2>/dev/null || cp -a $NDL $OUT   # hard links (no extra space) when on the same file system
cd G4NDL4.5
find . \( -name "18_36_Argon*" -o -name "18_38_Argon*" \) | while read -r f; do
  t=../$OUT/$f
  [ -e "$t" ] && continue       # this G4NDL already has the file
  mkdir -p "$(dirname "$t")" && cp -p "$f" "$t"
done
echo "added $(find ../$OUT -name '18_3[68]_Argon*' | wc -l) Ar-36/38 files to $EXTRA/$OUT"
