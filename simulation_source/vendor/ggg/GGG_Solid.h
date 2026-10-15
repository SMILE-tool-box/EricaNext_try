#ifndef GGG_SOLID_H_
#define GGG_SOLID_H_

#include "gggbasic.h"

class GGG_Solid {
  friend class GGG_Object;
  std::vector<G4ThreeVector> vertex_list;
  std::vector<GGG_LIB::FACE> face_list;
  std::string name;

 public:
  GGG_Solid();
  ~GGG_Solid();
  void add_vertex(const G4ThreeVector &vertex);
  void add_face(const GGG_LIB::FACE &face);
  void set_name(const std::string &name);

    G4ThreeVector get_center();

  int get_vertex_num();
  int get_face_num();
  std::string get_name();

  G4TessellatedSolid* create();
};

#endif  // GGG_SOLID_H_
