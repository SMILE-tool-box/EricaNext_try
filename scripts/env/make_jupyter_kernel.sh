#!/bin/bash
# Register the Jupyter kernel "EricaNext": /usr/bin/python3 with the environment of ~/opt/setup_ericanext.sh
# written into the kernel (kernel.json "env"), so notebooks run with it without changing ~/.bashrc.
set -euo pipefail
PY=${PYTHON:-/usr/bin/python3}
DIR=$(mktemp -d)
bash -c "source $HOME/opt/setup_ericanext.sh && $PY - '$DIR' '$PY'" <<'PYEOF'
import json, os, sys
d, py = sys.argv[1], sys.argv[2]
keys = ["PATH", "LD_LIBRARY_PATH", "PYTHONPATH", "RUBYLIB", "ANLNEXT_INSTALL", "COMPTONSOFT_INSTALL", "GEANT4_INSTALL", "ROOTSYS"]
keys += [k for k in os.environ if k.startswith("G4") and k.endswith("DATA")]
env = {k: os.environ[k] for k in keys if k in os.environ}
spec = {"argv": [py, "-m", "ipykernel_launcher", "-f", "{connection_file}"],
        "display_name": "EricaNext", "language": "python", "env": env}
json.dump(spec, open(os.path.join(d, "kernel.json"), "w"), indent=1)
PYEOF
jupyter kernelspec install --user --replace --name ericanext $DIR
rm -r $DIR
echo "kernel 'EricaNext' registered (jupyter kernelspec list)"
