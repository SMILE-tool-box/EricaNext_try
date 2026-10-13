#ifndef SMILE_Material_h
#define SMILE_Material_h 1

#include <globals.hh>
#include <G4Material.hh>
#include <G4MaterialTable.hh>
#include <G4Element.hh>
#include <G4ElementTable.hh>
#include <G4NistManager.hh>

using namespace std;

class SMILE_Material {
 public:
  SMILE_Material();
  SMILE_Material(double);
  virtual ~SMILE_Material();

  G4Material* GetMaterialByName(string *mat_name);

  // GAS for TPC
  G4Material* Ar90Eth10()   {return Ar90E10;};
  G4Material* SMILE_1_gas() {return SMILE1_TPC_Gas;};
  G4Material* ILC_gas()     {return ilc_gas;};
  G4Material* P10_CF4()     {return p10_cf4;};
  G4Material* CF4_Pure()    {return cf4_tpc;};
  G4Material* NeArCF4_64()  {return nac_64;};
  G4Material* SMILE_IIgas() {return ilc_gas;}

  // SCINTI
  G4Material* GSO()         {return gso;};
  G4Material* GAGG()        {return gagg;};
  G4Material* BGO()         {return nist->FindOrBuildMaterial("G4_BGO");};
  G4Material* PWO()         {return nist->FindOrBuildMaterial("G4_PbWO4");};
  G4Material* PlasticSC()   {
    return nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
  };
  G4Material* BaSO4()       {return baso4;};

  // materials
  G4Material* Aluminium()   {return nist->FindOrBuildMaterial("G4_Al");};
  G4Material* Iron()        {return nist->FindOrBuildMaterial("G4_Fe");};
  G4Material* Copper()      {return nist->FindOrBuildMaterial("G4_Cu");};
  G4Material* Magnesium()   {return nist->FindOrBuildMaterial("G4_Mg");};
  G4Material* Germanium()   {return nist->FindOrBuildMaterial("G4_Ge");};
  G4Material* Silicon()     {return nist->FindOrBuildMaterial("G4_Si");};
  G4Material* Ar_gas()      {return ar_gas;};
  G4Material* Ne_gas()      {return ne_gas;};
  G4Material* Xe_gas()      {return nist->FindOrBuildMaterial("G4_Xe");};

  G4Material* Air()         {return nist->FindOrBuildMaterial("G4_AIR");};
  G4Material* Vacuum()      {return nist->FindOrBuildMaterial("G4_Galactic");};
  G4Material* Ethane()      {return nist->FindOrBuildMaterial("G4_ETHANE");};
  G4Material* Methane()     {return nist->FindOrBuildMaterial("G4_METHANE");};
  G4Material* CF4()         {return cf4;};
  G4Material* iC4H10()      {return ic4h10;};
  G4Material* CO2()         {
    return nist->FindOrBuildMaterial("G4_CARBON_DIOXIDE");
  };

  G4Material* Mylar()       {return nist->FindOrBuildMaterial("G4_MYLAR");};
  G4Material* Teflon()      {return nist->FindOrBuildMaterial("G4_TEFLON");};
  G4Material* Kapton()      {return nist->FindOrBuildMaterial("G4_KAPTON");};
  G4Material* PEEK()        {return peek;};
  G4Material* AlHoneyComb() {return Al_HoneyComb;};
  G4Material* Polyimide()   {return polyimide;};
  G4Material* LCP()         {return lcp;};
  G4Material* Epoxy()       {return epoxy;};
  G4Material* G10()         {return g10;};
  G4Material* SiO2()        {
    return nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
  };
  G4Material* Glass()       {
    return nist->FindOrBuildMaterial("G4_GLASS_PLATE");
  };
  G4Material* SUS()         {
    return nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  };
  G4Material* CuMesh()      {return mesh;};
  G4Material* Battery1()    {return battery1;};
  G4Material* Battery2()    {return battery2;};
  G4Material* PVT()         {return pvt;};
  G4Material* PVC()         {return pvc;};
  G4Material* ESR()         {return esr;};
  G4Material* Concrete()    {return concrete;};
  G4Material* Ne_CF4()      {return ne_cf4;};

 private:
  void   CreateMaterial(double);

  G4NistManager *nist;
  G4Material *Al_HoneyComb, *polyimide, *epoxy, *gso, *gagg;
  G4Material *cf4, *ic4h10, *g10, *Ar90E10, *SMILE1_TPC_Gas, *p10_cf4;
  G4Material *cf4_tpc, *ilc_gas, *ar_gas, *mesh, *baso4;
  G4Material *ne_gas, *nac_64, *peek, *lcp;
  G4Material *socl2, *NMC;
  G4Material *pvc;
  G4Material *battery1, *battery2, *pvt;
  G4Material *esr;
  G4Material *concrete;
  G4Material *ne_cf4;
};

#endif
