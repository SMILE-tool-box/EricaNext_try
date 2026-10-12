#!/usr/bin/env python3
#
# Run the SMILE-2+ chain on the whole 20190701/Per02 ground-calibration run
# with the reference parameters (config/smile2p_20190701_Per02.json), for the
# event-by-event comparison against
#   /home/onoda/hikaru3_analysis2/sm2+_cal_rootdata/20190701_Per02/haboc_tmp_0.root
# Same as:  python3 run_chain.py smile2p_20190701_Per02 -o per02_out.root
import os, subprocess, sys

here = os.path.dirname(os.path.abspath(__file__))
sys.exit(subprocess.call([sys.executable, os.path.join(here, "run_chain.py"),
                          "smile2p_20190701_Per02", "-o", "per02_out.root"] + sys.argv[1:]))
