#ifndef SMILE_2p_Geo_h
#define SMILE_2p_Geo_h 1

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <globals.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4UserLimits.hh>
#include <G4ThreeVector.hh>
#include <G4RotationMatrix.hh>

#include "SMILE_Material.h"
#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4VisAttributes.hh>

#include "HAPositionManager.h"
#include "GGG_Model.h"

using namespace std;


class SMILE_2p_Geo : public G4VUserDetectorConstruction {
public:
    SMILE_2p_Geo(SMILE_Material *, string &, int, HAPositionManager *);

    virtual ~SMILE_2p_Geo();

    G4VPhysicalVolume *Construct();

    G4LogicalVolume *logicTPC;
    vector<G4VPhysicalVolume *> physPSA;
    vector<G4Transform3D *> PSATrans;
    vector<G4VPhysicalVolume *> logicDraw;


private:
    string data_dir;
    int geo_flag;
    SMILE_Material *material;
    G4VPhysicalVolume *Phys_World;
    HAPositionManager *phpos;

};

#endif
