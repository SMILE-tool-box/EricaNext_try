set(SUB_DIRS
  SaveFile
  ReadData
  CalcFADC
  TPC_Calibration
  TOT_Skewness
  HA_CenterGravity
  HA_Calc
  Reconstruct
)

foreach(dir ${SUB_DIRS})
  set(CUR_DIR modules/${dir})
  include(${CUR_DIR}/sources.cmake)
  set(MOD_DIRS ${CUR_DIR}/ ${MOD_DIRS} )
endforeach()
