#ifndef BASIC_H_
#define BASIC_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

// Geant4 header files
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"
#include "G4QuadrangularFacet.hh"
#include "G4NistManager.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
//#include "G4VSensitiveDetector.hh"
//#include "G4SDManager.hh"
// #include "myfunc.h"

namespace GGG_LIB {
  typedef struct tag_VECTOR3D {
    double x, y, z;
  }VECTOR3D;
  
  typedef struct tag_EDGE{
    int start, end;
  }EDGE;
  
  bool operator < (const tag_EDGE &left, const tag_EDGE &right);
  
  typedef struct tag_FACE {
    std::vector<int> id;
  }FACE;
}


inline bool GGG_LIB::operator < (const tag_EDGE &left, const tag_EDGE &right) {
  if (left.start < right.start)return true;
  else if (left.start == right.start && left.end < right.end)return true;
  else return false;
}

#endif  // BASIC_H_
