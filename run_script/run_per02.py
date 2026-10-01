#!/usr/bin/env python3
#
# Run the SMILE-2+ chain on the real 20190701/Per02 ground-calibration run,
# with every parameter taken from the authoritative reference setting.mac:
#   /home/onoda/hikaru3_analysis2/sm2+_cal_rawdata/setting.mac
# so the output can be compared event-by-event against the reference
# ROOT file at
#   /home/onoda/hikaru3_analysis2/sm2+_cal_rootdata/20190701_Per02/haboc_tmp_0.root

import anlnext
import EricaPy as th

RAW_DIR   = "/home/onoda/hikaru3_analysis2/sm2+_cal_rawdata/Per02_ETCC"
PARAM_DIR = "/home/onoda/hikaru3_analysis2/sm2+_cal_rawdata/param/20190528"

def setup(module):
    module.chain(th.SaveFile)
    module.with_parameters({
        "filename": "per02_out.root"
    })

    module.chain(th.ReadData)
    module.with_parameters({
        "data_dir": RAW_DIR,
        "reverse_TPC": True
    })

    # Chain order matches setting.mac's [Def] exactly:
    #   RawRead2p, CalcFADC, TPC_Calibration, HA_Center_Gravity, HA_Calc_s2,
    #   TOT_Skewness_Analysis, Reconstruct
    # (SaveFile/ReadData above stand in for RawRead2p + the SaveFile
    # infrastructure, which isn't a separate module in the reference chain).

    module.chain(th.CalcFADC)
    module.with_parameters({
        # [CalcFADC] Base_max - Base_min = 30, Charge_threshold : 20
        "base_window": 30.0,
        "threshold": 20.0
    })

    module.chain(th.TPC_Calibration)
    module.with_parameters({
        # [TPC_Calibration]
        "ParameterFile": PARAM_DIR + "/Calibration20190130.dat",
        # setting.mac says 1.4530463, but haboc passes parameters through a
        # TGNumberEntry GUI field that rounds to 6 significant digits; the
        # value actually used (recorded in HabocStatus.scl) is 1.45305.
        "Cal_Modify":    1.45305
    })

    module.chain(th.HA_CenterGravity)
    module.with_parameters({
        # [HA_Center_Gravity] ADC_Threshold : 10
        "threshold": 10,
        "overflow": 4000
    })

    module.chain(th.HA_Calc)
    module.with_parameters({
        # [HA_Calc_s2]
        "Cal_Mode": False,                       # Flag_Cal_Mode : 0
        "parameter_dir": PARAM_DIR,
        "Flag_Modify_PMT_Gain": True,             # Flag_Modify_PMT_Gain : 1
        "Modified_Parameter_Data_Dir": PARAM_DIR, # Modified_Parameter_Data_Dir
        "SingleHit_Eth": 50.0,                    # SingleHit_Eth(keV)
        "SingleHit_ADCth": 1000.0                 # SingleHit_ADCthth(ADC) (typo in setting.mac; code default == this anyway)
    })

    module.chain(th.TOT_Skewness)
    module.with_parameters({
        # [TOT_Skewness_Analysis]
        "drift_velocity_file": RAW_DIR + "/drift_velocity.dat"
    })

    module.chain(th.Reconstruct)

a = anlnext.AnalysisChain()
setup(a)
a.run(-1)
