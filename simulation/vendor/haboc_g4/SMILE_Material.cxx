#include "SMILE_Material.h"
#include <G4SystemOfUnits.hh>

G4Material* SMILE_Material::GetMaterialByName(string *mat_name){
  G4Material* mat=0;

  if(*mat_name == "A7075" ||
     *mat_name == "A6063" ||
     *mat_name == "A5052" ||
     *mat_name == "A5083" ||
     *mat_name == "Alumi")                return Aluminium();
  else if(*mat_name == "Fe")              return Iron();
  else if(*mat_name == "SUS304")          return SUS();
  else if(*mat_name == "PEEK")            return PEEK();
  else if(*mat_name == "LCP")             return LCP();
  else if(*mat_name == "G10_Plate")       return G10();
  else if(*mat_name == "teflon")          return Teflon();
  else if(*mat_name == "Copper")          return Copper();
  else if(*mat_name == "GSO")             return GSO();
  else if(*mat_name == "TPC_gas")         return ILC_gas();
  else if(*mat_name == "Polyimide")       return Polyimide();
  else if(*mat_name == "quartz")          return Glass();
  else if(*mat_name == "Galactic")        return Vacuum();
  else if(*mat_name == "Air")             return Air();
  else if(*mat_name == "battery1")        return Battery1();
  else if(*mat_name == "battery2")        return Battery2();
  else if(*mat_name == "PVT")             return PVT();
  else if(*mat_name == "PVC")             return PVC();
  else if(*mat_name == "ESR")             return ESR();
  else if(*mat_name == "Concrete")        return Concrete();
  else if(*mat_name == "Ne_CF4") return Ne_CF4();
  return mat;
}

void SMILE_Material::CreateMaterial(double pressure){
  nist = G4NistManager::Instance();
  nist->PrintElement("all");

  Al_HoneyComb
    = new G4Material("AlHoneyComb", 0.135*g/cm3, 1);
  Al_HoneyComb->AddElement(nist->FindOrBuildElement("Al"), 1);

  mesh = new G4Material("Cu_mesh", 0.3 * 8.96*g/cm3, 1);
  mesh->AddElement(nist->FindOrBuildElement("Cu"), 1);



  socl2 = new G4Material("socl2", 1.64 * g/cm3, 3);
  socl2->AddElement(nist->FindOrBuildElement("S"), 1);
  socl2->AddElement(nist->FindOrBuildElement("O"), 1);
  socl2->AddElement(nist->FindOrBuildElement("Cl"), 2);

  //concrete for the ceiling and floor  20190617 by Yoshikawa
	concrete = new G4Material("Concrete", 2.5 * g/cm3, 6);
  concrete->AddElement(nist->FindOrBuildElement("O"), 0.52);
  concrete->AddElement(nist->FindOrBuildElement("Si"), 0.325);
  concrete->AddElement(nist->FindOrBuildElement("Ca"), 0.06);
  concrete->AddElement(nist->FindOrBuildElement("Na"), 0.015);
  concrete->AddElement(nist->FindOrBuildElement("Fe"), 0.04);
  concrete->AddElement(nist->FindOrBuildElement("Al"), 0.04);


  // powder
  NMC = new G4Material("NMC", 1. * g/cm3, 5);
  NMC->AddElement(nist->FindOrBuildElement("Li"), 10);
  NMC->AddElement(nist->FindOrBuildElement("Ni"), 4);
  NMC->AddElement(nist->FindOrBuildElement("Mn"), 3);
  NMC->AddElement(nist->FindOrBuildElement("Co"), 3);
  NMC->AddElement(nist->FindOrBuildElement("O"), 20);

  pvt = new G4Material("PVT",1.032*g / cm3, 2);
  pvt->AddElement(nist->FindOrBuildElement("C"), 9);
  pvt->AddElement(nist->FindOrBuildElement("H"), 10);

  pvc = new G4Material("PVC", 1.45*g / cm3, 3);
  pvc->AddElement(nist->FindOrBuildElement("C"),2);
  pvc->AddElement(nist->FindOrBuildElement("H"),3);
  pvc->AddElement(nist->FindOrBuildElement("Cl"),1);

  polyimide
    = new G4Material("Polyimide", 1.47*g/cm3, 4);
  polyimide->AddElement(nist->FindOrBuildElement("C"), 14);
  polyimide->AddElement(nist->FindOrBuildElement("H"), 22);
  polyimide->AddElement(nist->FindOrBuildElement("O"),  4);
  polyimide->AddElement(nist->FindOrBuildElement("N"),  2);

  epoxy
    = new G4Material("Epoxy", 1.7*g/cm3 , 3);
  epoxy->AddElement(nist->FindOrBuildElement("C"), 10);
  epoxy->AddElement(nist->FindOrBuildElement("H"), 10);
  epoxy->AddElement(nist->FindOrBuildElement("O"),  2);

  peek
    = new G4Material("PEEK", 1.32*g/cm3, 3);
  peek->AddElement(nist->FindOrBuildElement("C"), 13);
  peek->AddElement(nist->FindOrBuildElement("H"),  8);
  peek->AddElement(nist->FindOrBuildElement("O"),  2);

  lcp
    = new G4Material("LCP", 1.35*g/cm3, 3);
  lcp->AddElement(nist->FindOrBuildElement("C"),  7);
  lcp->AddElement(nist->FindOrBuildElement("H"),  4);
  lcp->AddElement(nist->FindOrBuildElement("O"),  2);

  gso
    = new G4Material("GSO", 6.71*g/cm3, 3);
  gso->AddElement(nist->FindOrBuildElement("Gd"),  2);
  gso->AddElement(nist->FindOrBuildElement("Si"),  1);
  gso->AddElement(nist->FindOrBuildElement("O"),   5);

  gagg
    = new G4Material("GAGG", 6.63*g/cm3, 4);
  gagg->AddElement(nist->FindOrBuildElement("Gd"), 3);
  gagg->AddElement(nist->FindOrBuildElement("Al"), 2);
  gagg->AddElement(nist->FindOrBuildElement("Ga"), 3);
  gagg->AddElement(nist->FindOrBuildElement("O"), 12);

  g10 =
    new G4Material("G10", 2.0*g/cm3,  2);
  g10->AddMaterial(SiO2(), 60*perCent);
  g10->AddMaterial(Epoxy(), 40*perCent);

  ar_gas = nist->BuildMaterialWithNewDensity("Ar_gas", "G4_Ar", 1.784*mg/cm3);
  ne_gas = nist->BuildMaterialWithNewDensity("Ne_gas", "G4_Ne", 0.9002*mg/cm3);

  cf4
    = new G4Material("CF4", 3.884*mg/cm3, 2,
		     kStateGas, 273.*kelvin, 1.*atmosphere);
  cf4->AddElement(nist->FindOrBuildElement("C"), 1);
  cf4->AddElement(nist->FindOrBuildElement("F"), 4);

  ic4h10
    = new G4Material("iC4H10", 2.673*mg/cm3, 2,
		     kStateGas, 273*kelvin, 1.*atmosphere);
  ic4h10->AddElement(nist->FindOrBuildElement("C"),  4);
  ic4h10->AddElement(nist->FindOrBuildElement("H"), 10);

  baso4 = new G4Material("BaSO4", 4.50*g/cm3, 3);
  baso4->AddElement(nist->FindOrBuildElement("Ba"), 1);
  baso4->AddElement(nist->FindOrBuildElement("S"), 1);
  baso4->AddElement(nist->FindOrBuildElement("O"), 4);

  ///////////////////////////////////////////////////////////
  ////     TPC gases    /////////////////////////////////////
  ///////////////////////////////////////////////////////////
  Ar90E10 = new G4Material("Ar90_Ethane10",
			   1.729*mg/cm3 * pressure,
			   2, kStateGas,
			   300.*kelvin,
			   pressure * atmosphere);
  Ar90E10->AddMaterial(Ar_gas(), 90.*perCent);
  Ar90E10->AddMaterial(Ethane(), 10.*perCent);

  SMILE1_TPC_Gas = new G4Material("SMILE1_TPC_Gas",
				  3.963*mg/cm3 * pressure,
				  3, kStateGas,
				  300.*kelvin,
				  pressure * atmosphere);
  SMILE1_TPC_Gas->AddMaterial(Xe_gas(), 80.*perCent);
  SMILE1_TPC_Gas->AddMaterial(Ar_gas(), 18.*perCent);
  SMILE1_TPC_Gas->AddMaterial(Ethane(),  2.*perCent);

  p10_cf4 = new G4Material("P10_CF4",
			   2.336*mg/cm3 * pressure,
                           3, kStateGas,
                           300*kelvin,
			   pressure * atmosphere);
  p10_cf4->AddMaterial(CF4(), 40*perCent);
  p10_cf4->AddMaterial(Ar_gas(), 54*perCent);
  p10_cf4->AddMaterial(iC4H10(), 6*perCent);

  cf4_tpc = new G4Material("CF4_Pure",
			   3.884*mg/cm3 * pressure,
			   2, kStateGas,
			   300.*kelvin,
			   pressure * atmosphere);
  cf4_tpc->AddElement(nist->FindOrBuildElement("C"), 1);
  cf4_tpc->AddElement(nist->FindOrBuildElement("F"), 4);

  ilc_gas = new G4Material("ILC_Gas",
			   1.865*mg/cm3 * pressure,
			   3, kStateGas,
			   300*kelvin,
			   pressure * atmosphere);
  ilc_gas->AddMaterial(Ar_gas(), 90.885*perCent);
  ilc_gas->AddMaterial(CF4(),     6.248*perCent);
  ilc_gas->AddMaterial(iC4H10(),  2.867*perCent);

  nac_64 = new G4Material("NAC_64",
			  2.096*mg/cm3 * pressure,
			  3, kStateGas, 300*kelvin,
			  pressure * atmosphere);
  nac_64->AddMaterial(Ne_gas(),  25.636*perCent);
  nac_64->AddMaterial(Ar_gas(),   0.255*perCent);
  nac_64->AddMaterial(CF4(),     74.109*perCent);


  ne_cf4 = new G4Material("Ne_CF4",
       2.392*mg/cm3 * pressure,
       2, kStateGas, 300*kelvin,
       pressure * atmosphere);
  ne_cf4->AddMaterial(Ne_gas(),  50.0*perCent);
  ne_cf4->AddMaterial(CF4(),     50.0*perCent);


  ///////////////////////////////////////////////////////////
  ////     battery    /////////////////////////////////////
  ///////////////////////////////////////////////////////////
  battery1 = NMC;
  battery2 = socl2;


  esr = new G4Material("ESR",1.35*g/cm3, 3);
  esr->AddElement(nist->FindOrBuildElement("C"), 10);
  esr->AddElement(nist->FindOrBuildElement("H"),  8);
  esr->AddElement(nist->FindOrBuildElement("O"),  4);

}

SMILE_Material::SMILE_Material(){
  CreateMaterial(1.);
}

SMILE_Material::SMILE_Material(double pressure){
  CreateMaterial(pressure);
}

SMILE_Material::~SMILE_Material(){
}
