#include "GGG_Solid.h"

using namespace GGG_LIB;

GGG_Solid::GGG_Solid(){
}

GGG_Solid::~GGG_Solid(){
}

//---------------------------------------------------
//
//  set variable
//
//---------------------------------------------------
void GGG_Solid::add_vertex(const G4ThreeVector &vertex) {
  vertex_list.push_back(vertex);
}


void GGG_Solid::add_face(const FACE &face) {
  face_list.push_back(face);
}

void GGG_Solid::set_name(const std::string &Name) {
  this->name = Name;
}

//---------------------------------------------------
//
//  get variable
//
//---------------------------------------------------
int GGG_Solid::get_vertex_num() {
  return vertex_list.size();
}

int GGG_Solid::get_face_num() {
  return face_list.size();
}

std::string GGG_Solid::get_name() {
  return name;
}

G4TessellatedSolid* GGG_Solid::create() {
  int fnum = get_face_num();
  int vnum;
  G4TessellatedSolid* solidTarget = new G4TessellatedSolid(name.c_str());
  G4VFacet* facet;
  
  for (int i = 0; i < fnum; i++) {
    vnum = face_list[i].id.size();
    switch (vnum) {
    case 3:
      facet =  new 
	G4TriangularFacet(vertex_list[face_list[i].id[2]],
			  vertex_list[face_list[i].id[1]],
			  vertex_list[face_list[i].id[0]],
			  ABSOLUTE);
      
      solidTarget->AddFacet((G4VFacet*)facet);
      break;
      
    case 4:
      facet = new
	G4QuadrangularFacet(vertex_list[face_list[i].id[3]],
			    vertex_list[face_list[i].id[2]],
			    vertex_list[face_list[i].id[1]],
			    vertex_list[face_list[i].id[0]],
			    ABSOLUTE);
      
      solidTarget->AddFacet((G4VFacet*)facet);
      break;
      
    default:
      
      break;
    }
  }

  solidTarget->SetSolidClosed(true);
  return solidTarget;
}


G4ThreeVector GGG_Solid::get_center(){
    G4ThreeVector sol(0,0,0);

    for(int i=0;i<get_vertex_num();i++){
        sol += vertex_list[i];
    }
    return sol/get_vertex_num();
}