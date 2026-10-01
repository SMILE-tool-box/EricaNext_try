%module EricaPy
%{
#include <anlnext/BasicModule.hh>

// include headers of my modules
#include "SaveFile.h"
#include "ReadData.h"
#include "CalcFADC.h"
#include "TPC_Calibration.h"
#include "TOT_Skewness.h"
#include "HA_CenterGravity.h"
#include "HA_Calc.h"
#include "Reconstruct.h"

%}

%import(module="anlnext.anlnextpy") "anlnext/python/anlnextpy.i"


// interface to my modules

class SaveFile         : public anlnext::BasicModule {};
class ReadData         : public anlnext::BasicModule {
 public:
  int GetMinAnode() const;
  int GetMaxAnode() const;
  int GetMinCathode() const;
  int GetMaxCathode() const;
  int GetMinClock() const;
  int GetMaxClock() const;
  unsigned Get_EventID() const;
  unsigned Get_SEvtID() const;
};
class CalcFADC         : public anlnext::BasicModule {
 public:
  double Get_Qanode(int ch) const;
  double Get_Qcathode(int ch) const;
};
class TPC_Calibration  : public anlnext::BasicModule {
 public:
  double Get_Energy() const;
  int Get_NCoinci() const;
  int Get_MaxPanelHit() const;
  int Get_Na() const;
  int Get_Nc() const;
  int Get_HitPanel(int i, int j) const;
};
class TOT_Skewness     : public anlnext::BasicModule {};
class HA_CenterGravity : public anlnext::BasicModule {};
class HA_Calc          : public anlnext::BasicModule {};
class Reconstruct      : public anlnext::BasicModule {};
