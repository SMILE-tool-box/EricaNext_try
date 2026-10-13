#ifndef GGG_OBJECT_H_
#define GGG_OBJECT_H_

#include <TMath.h>
#include "GGG_Solid.h"
#include "SMILE_Material.h"

class GGG_Object {
  std::vector<GGG_Solid> solid_list;
  std::vector<G4LogicalVolume*> LogicalVolume_list;
  bool visible;
  std::string material_name;
  std::string object_name;

  void erase_solid(int id) { solid_list.erase(solid_list.begin()+id); }
	

 public:
  GGG_Object();
  ~GGG_Object();

  void add_solid(const GGG_Solid solid);
  void set_objmaterial_name(const std::string name);
  void set_object_name(const std::string name);
  void set_visible(const bool flag);
  //template<typename G4VSD> 
  //void set_sensitive_detector(const std::string sd_header, 
  //				G4VSD** aValue, G4SDManager* SDmanager);

  int         get_solid_num()       { return solid_list.size(); }
  std::string get_objmaterial_name(){ return material_name; }
  std::string get_object_name()     { return object_name; }
  bool        get_visible()         { return visible; }
  G4LogicalVolume* get_logicvol(int id=0) { return LogicalVolume_list[id]; }

  void divide_solid();
  //void create(G4NistManager* nist, G4LogicalVolume* logic_world,
  void create(SMILE_Material* material, G4LogicalVolume* logic_world,
	      G4RotationMatrix* rotation, G4ThreeVector &position, 
	      G4bool checkOverlaps);
};


//template<typename G4VSD> void GGG_Object::set_sensitive_detector
//(const std::string sd_header, G4VSD** aValue, G4SDManager* SDmanager) {
//int sol_num = get_solid_num();
//std::string sdname = sd_header;
//aValue = new G4VSD*[sol_num];
//
//for (int i = 0; i < sol_num; i++) {
//  sdname += "_" + to_string(i);
//  aValue[i] = new G4VSD(sdname);
//  
//  LogicalVolume_list[i]->SetSensitiveDetector(aValue[i]);
//  SDmanager->AddNewDetector(aValue[i]);
//}
//}

#endif  // GGG_OBJECT_H_
