#include "GGG_Object.h"

#include <map>

//---------------------------------------------------
//
//  local function
//
//---------------------------------------------------

//template<typename T>  T min(T a, T b) {
//if (a < b)return a;
//else return b;
//}
//template<typename T>  T min(T a, T b, T c) {
//T val = min(a, b);
//return min(val, c);
//}
//template<typename T>  T min(T a, T b, T c, T d) {
//T v0 = min(a, b);
//T v1 = min(v0, c);
//return min(v1, d);
//}

//template<typename T>  T max(T a, T b) {
//if (a > b)return a;
//else return b;
//}
//template<typename T>  T max(T a, T b, T c) {
//return max(max(a, b), c);
//}
//template<typename T>  T max(T a, T b, T c, T d) {
//return max(max(max(a, b), c), d);
//}



//---------------------------------------------------
//
//  
//
//---------------------------------------------------
GGG_Object::GGG_Object(){
}


GGG_Object::~GGG_Object(){
}


//---------------------------------------------------
//
//  set variable
//
//---------------------------------------------------
void GGG_Object::add_solid(const GGG_Solid solid) {
  solid_list.push_back(solid);
}
void GGG_Object::set_objmaterial_name(const std::string name) {
  material_name = name;
}
void GGG_Object::set_object_name(const std::string name) {
  object_name = name;
}

void GGG_Object::set_visible(const bool flag) {
  visible = flag;
}


//---------------------------------------------------
//
//
//
//---------------------------------------------------
//void GGG_Object::create(G4NistManager* nist, G4LogicalVolume* logic_world, 
void GGG_Object::create(SMILE_Material* material, G4LogicalVolume* logic_world, 
			G4RotationMatrix* rotation, G4ThreeVector &position, 
			G4bool checkOverlaps) {
  int sol_num = get_solid_num();
  G4Material* mat = 0;
  G4TessellatedSolid* solid;
  
  for (int i = 0; i < sol_num; i++) {
    mat = material->GetMaterialByName(&material_name);
    solid = solid_list[i].create();
    std::cout << mat->GetName() << endl;

    G4LogicalVolume* shape = new G4LogicalVolume(solid,
						 mat,
						 solid->GetName());
    
    new G4PVPlacement(rotation,
		      position,
		      shape,
		      solid->GetName(),
		      logic_world,
		      false,
		      0,
		      checkOverlaps);
    
    LogicalVolume_list.push_back(shape);
    G4cout << "solid: " << solid->GetName() << " was created (GGG)" << G4endl;
  }
  G4cout << "object: " << get_object_name() << " was created (GGG)" << G4endl;
}


//---------------------------------------------------
//
// private
//
//---------------------------------------------------


void GGG_Object::divide_solid() {
  std::vector<int> v_color;
  std::vector<int> v_color_list;

  //-----errors-----
  if (solid_list.size() > 1) {
    G4cout << "Error: solid is already divided." << G4endl;
    return;
  }
  else if(solid_list.size()==0){
    G4cout << "Error: solid is not found." << G4endl;
    return;
  }
  
  //-----set color-----
  for (int i = 0; i < solid_list[0].get_vertex_num(); i++) {
    v_color.push_back(i);
  }
  
  bool changed;
  GGG_LIB::FACE f;
  G4int minimum=0, maximum=0;
  do{
    changed = false;
    for (int i = 0; i < solid_list[0].get_face_num(); i++) {		
      f = solid_list[0].face_list[i];
      
      //G4cout << f.id[0] <<": " << f.id[1] << ": " << f.id[2] << G4endl;(debug)
      if (f.id.size() == 3) {
	minimum = TMath::Min(TMath::Min(v_color[f.id[0]], v_color[f.id[1]]), 
			     v_color[f.id[2]]);
	maximum = TMath::Max(TMath::Max(v_color[f.id[0]], v_color[f.id[1]])
			     , v_color[f.id[2]]);
      }
      else if(f.id.size() == 4) {
	minimum
	  = TMath::Min(TMath::Min(TMath::Min(v_color[f.id[0]], 
					     v_color[f.id[1]]),
				  v_color[f.id[2]]),
		       v_color[f.id[3]]);
	maximum 
	  = TMath::Max(TMath::Max(TMath::Max(v_color[f.id[0]], 
					     v_color[f.id[1]]), 
				  v_color[f.id[2]])
		       , v_color[f.id[3]]);
      }
      else {
	G4cout << "error: face num is illegal" << G4endl;
      }
      
      if (minimum != maximum) {
	for (unsigned j = 0; j < f.id.size(); j++) {
	  v_color[f.id[j]] = minimum;
	}
	changed = true;
      }
    }
  } while (changed == true);
  
  
  //-----get color list-----
  for (int i = 0; i < solid_list[0].get_vertex_num(); i ++) {
    auto clf = std::find(v_color_list.begin(), v_color_list.end(), v_color[i]);
    if (clf == v_color_list.end()) {
      v_color_list.push_back(v_color[i]);
    }
  }
  
  //-----divid solid-----
  
  
  G4cout << "There is " << v_color_list.size() 
	 << " parts in " << solid_list[0].get_name() << G4endl;
  if (v_color_list.size() == 1) {
    G4cout << "dividing is not needed in " 
	   << solid_list[0].get_name() << G4endl;
    return;
  }
  
  for (unsigned col = 0; col < v_color_list.size(); col++) {
    GGG_Solid solid_buf;	
    std::map<int, int> v_map;//old id to new id
    
    //----set vertex-----
    for (int ver = 0; ver < solid_list[0].get_vertex_num(); ver++) {
      if (v_color[ver] == v_color_list[col]) {
	solid_buf.add_vertex(solid_list[0].vertex_list[ver]);
	v_map.insert(std::make_pair(ver, v_map.size()));
      }
    }
    //----set face-----
    for (int face = 0; face < solid_list[0].get_face_num(); face++) {
      GGG_LIB::FACE face_buf;
      if (v_color[solid_list[0].face_list[face].id[0]] == v_color_list[col]) {
	for(unsigned ver=0; ver<solid_list[0].face_list[face].id.size();ver++) {
	  face_buf.id.push_back(v_map.at(solid_list[0]
					 .face_list[face].id[ver]));
	}
	solid_buf.add_face(face_buf);
      }
    }
    //----set name-----
    std::string name = solid_list[0].get_name();
    name += "_" + std::to_string(col);
    solid_buf.set_name(name);
    
    add_solid(solid_buf);
  }
  erase_solid(0);
}
