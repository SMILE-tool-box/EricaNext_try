/*!
 * @date 2019/10/15 the geometry of the side ESR, teflon and black tapes were modified.
 */

//




#include "SMILE_2p_Geo.h"
#include <G4SystemOfUnits.hh>
#include <G4VNestedParameterisation.hh>

#include <CLHEP/Geometry/Transform3D.h>

#include <HA_common.h>

using namespace HA_common;


const int GEO_FLAG_GROUND = 0;
const int GEO_FLAG_FLIGHT = 1;
const int GEO_FLAG_NO_VESSEL = 2;

SMILE_2p_Geo::SMILE_2p_Geo(SMILE_Material *mat, string &dir, int gflag, HAPositionManager *thishpos) {
    material = mat;
    data_dir = dir;
    geo_flag = gflag;
    phpos = thishpos;
}

SMILE_2p_Geo::~SMILE_2p_Geo() {
}


G4VPhysicalVolume *SMILE_2p_Geo::Construct() {
    const double World_Size_XY = 8*meter;
    const double World_Size_Z  = 8*meter;
    //const double World_Size_XY = 6*meter;
    //const double World_Size_Z  = 8*meter;
    string  mat_name = "Galactic";
    if (geo_flag == GEO_FLAG_GROUND)  mat_name = "Air";

    // world
    G4Box *solid_world = new G4Box("World", World_Size_XY / 2, World_Size_XY / 2, World_Size_Z / 2);
    G4LogicalVolume *logic_world
            = new G4LogicalVolume(solid_world,
                                  material->GetMaterialByName(&mat_name),
                                  "World");
    Phys_World = new G4PVPlacement(0,
                                   G4ThreeVector(),
                                   logic_world,
                                   "World",
                                   0,
                                   false,
                                   0, 1);
    logic_world->SetVisAttributes(G4VisAttributes::GetInvisible() /* EricaSim: Geant4 11 API */);

    string geo_data_name;

    //convert coordinate (.mqo->G4)
    GGG_Model::set_convert_default(1, 3, -2);
    GGG_Model::set_position_default(0, 0, 476.);



    string name = "ceiling";
    if (geo_flag == GEO_FLAG_GROUND) {
        //const double CEILING_Z = -1650.  * mm;	//debug
        const double CEILING_Z = -2423.  * mm;	//ETCC on the green hand cart
        const double FLOOR_Z   =  1285.  * mm;	//ETCC on the green hand cart
		    const double CEILING_THICKNESS = 1000 * mm;
  	    const double FLOOR_THICKNESS   = 1000 * mm;
  	    const double BEAM_THICKNESS_1  = 1090 * mm;
  	    const double BEAM_X_1          = 1595 * mm;
  	    const double BEAM_THICKNESS_2  = 790 * mm;
  	    const double BEAM_Y_2          = 1980 * mm;
  	    const double BEAM_THICKNESS_3  = 550 * mm;
  	    const double BEAM_WIDTH_3      = 440 * mm;
        const double WALL_THICKNESS    = 500 * mm;
        mat_name = "Concrete";

        name = "ceiling";
        G4Box *solid_ceiling
            = new G4Box(name,  World_Size_XY / 2, World_Size_XY / 2,  CEILING_THICKNESS / 2);

        G4LogicalVolume *logic_ceiling
            = new G4LogicalVolume(solid_ceiling,
                                  material->GetMaterialByName(&mat_name),
                                  name);

        G4VPhysicalVolume *aBoxPhys
				    = new G4PVPlacement(0,
                                G4ThreeVector(0.,0., CEILING_Z - CEILING_THICKNESS / 2 ),
                                logic_ceiling,
                                name.c_str(),
                                logic_world,
                                false,
                                0);

        name = "ceiling_beam_1";
        G4Box *solid_beam_1
            = new G4Box(name, ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_X_1 ) / 2 , World_Size_XY / 2 - WALL_THICKNESS, BEAM_THICKNESS_1 / 2);

        G4LogicalVolume *logic_beam_1
            = new G4LogicalVolume(solid_beam_1,
                                  material->GetMaterialByName(&mat_name),
                                  name);

        aBoxPhys
				    = new G4PVPlacement(0,
                                G4ThreeVector(BEAM_X_1 + ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_X_1 ) / 2, 0., CEILING_Z + BEAM_THICKNESS_1 / 2 ),
                                logic_beam_1,
                                name.c_str(),
                                logic_world,
                                false,
                                0);


        name = "ceiling_beam_2";
        G4Box *solid_beam_2
            = new G4Box(name,  World_Size_XY / 2 - WALL_THICKNESS - ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_X_1 ) / 2,  ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_Y_2 ) / 2, BEAM_THICKNESS_2 / 2);

        G4LogicalVolume *logic_beam_2
            = new G4LogicalVolume(solid_beam_2,
                                  material->GetMaterialByName(&mat_name),
                                  name);

        aBoxPhys
				    = new G4PVPlacement(0,
                                G4ThreeVector( - ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_X_1 ) / 2, BEAM_Y_2 + ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_Y_2 ) / 2, CEILING_Z + BEAM_THICKNESS_2 / 2 ),
                                logic_beam_2,
                                name.c_str(),
                                logic_world,
                                false,
                                0);


        name = "ceiling_beam_3";
        G4Box *solid_beam_3
            = new G4Box(name,   BEAM_WIDTH_3 / 2, World_Size_XY / 2 - WALL_THICKNESS - ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_Y_2 ) / 2, BEAM_THICKNESS_3 / 2);

        G4LogicalVolume *logic_beam_3
            = new G4LogicalVolume(solid_beam_3,
                                  material->GetMaterialByName(&mat_name),
                                  name);

        aBoxPhys
				    = new G4PVPlacement(0,
                                G4ThreeVector(-750. - BEAM_WIDTH_3 / 2, - ( World_Size_XY / 2 - WALL_THICKNESS - BEAM_Y_2 ) / 2, CEILING_Z + BEAM_THICKNESS_3 / 2 ),
                                logic_beam_3,
                                name.c_str(),
                                logic_world,
                                false,
                                0);


        name = "floor";
        G4Box *solid_floor
            = new G4Box(name,  World_Size_XY / 2, World_Size_XY / 2,  FLOOR_THICKNESS / 2);

        G4LogicalVolume *logic_floor
            = new G4LogicalVolume(solid_floor,
                                  material->GetMaterialByName(&mat_name),
                                  name);

        aBoxPhys
            = new G4PVPlacement(0,
                                G4ThreeVector(0.,0., FLOOR_Z + FLOOR_THICKNESS / 2 ),
                                logic_floor,
                                name.c_str(),
                                logic_world,
                                false,
                                0);


        for (int i = 0; i < 4; i++ ){
		        ostringstream wall_num;
						wall_num << i;
            name = "wall_" + wall_num.str();

            G4Box *solid_wall
                = new G4Box(name,  World_Size_XY / 2 - WALL_THICKNESS / 2,  WALL_THICKNESS / 2, (FLOOR_Z - CEILING_Z) / 2);


            G4LogicalVolume *logic_wall
                = new G4LogicalVolume(solid_wall,
                                      material->GetMaterialByName(&mat_name),
                                      name);

            G4RotationMatrix* rot_wall = new G4RotationMatrix();
						rot_wall -> rotateZ( 90.0 * i * deg);
            G4ThreeVector lot_wall(WALL_THICKNESS / 2, World_Size_XY / 2 - WALL_THICKNESS / 2, (FLOOR_Z + CEILING_Z) / 2 );
						lot_wall.rotate(90.0 * i * deg, 0 , 0);

            aBoxPhys
                = new G4PVPlacement(rot_wall,
                                    lot_wall,
                                    logic_wall,
                                    name.c_str(),
                                    logic_world,
                                    false,
                                    0);
				}

        mat_name = "Fe";
        name = "L_angle_1";
        G4Box *solid_L_angle_1
            = new G4Box(name,  3000. / 2,  30. / 2,  2. / 2);

        G4LogicalVolume *logic_L_angle_1
            = new G4LogicalVolume(solid_L_angle_1,
                                  material->GetMaterialByName(&mat_name),
                                  name);

        aBoxPhys
            = new G4PVPlacement(0,
                                G4ThreeVector(0.,0., -1628),
                                logic_L_angle_1,
                                name.c_str(),
                                logic_world,
                                false,
                                0);

        name = "L_angle_2";
        G4Box *solid_L_angle_2
            = new G4Box(name,  3000. / 2,  2. / 2,  28. / 2);

        G4LogicalVolume *logic_L_angle_2
            = new G4LogicalVolume(solid_L_angle_2,
                                  material->GetMaterialByName(&mat_name),
                                  name);

        aBoxPhys
            = new G4PVPlacement(0,
                                G4ThreeVector(0., 15., -1628-15),
                                logic_L_angle_2,
                                name.c_str(),
                                logic_world,
                                false,
                                0);

    }




    GGG_Model pressured_vessel;
    GGG_Model gondola;

    G4LogicalVolume *pBlstLog[4];

    //####### make Gondola #########//
    if (geo_flag == GEO_FLAG_FLIGHT) {
        geo_data_name = data_dir + "/bCALET.mqo";
        pressured_vessel.load(geo_data_name);

        geo_data_name = data_dir + "/gondola.mqo";
        gondola.load(geo_data_name);


        pressured_vessel.create(material, logic_world);
        gondola.create(material, logic_world, GGG_MODEL_DIVIDE);
        for (int rotid = 0; rotid < 4; rotid++) {
            string name = "blst_" + to_string(rotid);
            string mat_name = "Fe";

            G4VSolid *pBoxSolid =
                    new G4Box(name,
                              300. / 2. * mm, 300. / 2. * mm, 115. / 2. * mm);

            pBlstLog[rotid] =
                    new G4LogicalVolume(pBoxSolid,
                                        material->GetMaterialByName(&mat_name),
                                        name);

            G4ThreeVector pos_blst(720. / TMath::Sqrt(2) * mm, 720. / TMath::Sqrt(2) * mm, (476. + 1300.) * mm);
            //G4ThreeVector pos_blst(720. * mm, 0, (476. + 1300.) * mm); 2019/07/05 Yoshikawa
            G4RotationMatrix *rot_mat = new G4RotationMatrix;
            rot_mat->rotateZ(90. * deg * rotid);
            pos_blst = (*rot_mat) * pos_blst;

            G4VPhysicalVolume *aBoxPhys =
                    new G4PVPlacement(0,
                                      pos_blst,
                                      pBlstLog[rotid],
                                      name,
                                      logic_world,
                                      false,
                                      0);
        }
    }

    //###### make pressure vessel #########//
    geo_data_name = data_dir + "/air.mqo";
    GGG_Model vessel_inside(geo_data_name);
    if (geo_flag == GEO_FLAG_NO_VESSEL) {
        vessel_inside.set_material("Air", "Galactic");
    }
    vessel_inside.create(material, logic_world);
    if (geo_flag == GEO_FLAG_NO_VESSEL || geo_flag == GEO_FLAG_GROUND) {
        vessel_inside.get_logicvol("Air")->SetVisAttributes(G4VisAttributes::GetInvisible() /* EricaSim: Geant4 11 API */);
    }

    //##### make anti ############################//
    geo_data_name = data_dir + "/etcc_anti.mqo";
    GGG_Model etcc_anti(geo_data_name);
    if (geo_flag == GEO_FLAG_NO_VESSEL) {
      etcc_anti.set_material("PVT","Air");
    }
    etcc_anti.create(material, vessel_inside.get_logicvol("Air"));
    if (geo_flag == GEO_FLAG_NO_VESSEL ) {
        etcc_anti.get_logicvol("Air")->SetVisAttributes(G4VisAttributes::GetInvisible() /* EricaSim: Geant4 11 API */);
    }

    //######## make ETCC vessel ###############//
    GGG_Model tpc_vessel;
    if (geo_flag != GEO_FLAG_NO_VESSEL) {
        geo_data_name = data_dir + "/etcc_vessel.mqo";
        tpc_vessel.load(geo_data_name);

        tpc_vessel.create(material, vessel_inside.get_logicvol("Air"));
    }

    //####### gas volume inside ETCC vessel #######//
    geo_data_name = data_dir + "/tpc_gas.mqo";
    GGG_Model tpc_gas_volume(geo_data_name);
    tpc_gas_volume.create(material, vessel_inside.get_logicvol("Air"));



    const double ARR_DEL_X = 114.0 * mm;
    const double PX_DEL_XY = 6.5 * mm;
    const double PMT_DEL_XY = 54.0 * mm;

    //-----bottom-----
    const double BASE_BOTTOM_X = (166.75 - 3.) * mm;
    const double BASE_BOTTOM_Y = (166.75 - 3.) * mm;
    const double BASE_BOTTOM_Z = (476. - (134.9 + 108.9) / 2.) * mm;
    const double SCINTI_PIX_WIDTH = 6. * mm;
    const double BOTTOM_SCINTI_DEPTH = 26. * mm;

    const double ESR_DEPTH = 0.065 * mm;
    const double TEFLON_DEPTH = 0.2 * mm;
    const double TAPE_DEPTH = 0.22 * mm;

    mat_name = "GSO";
    name = "scinti_bottom";


    for (int haid = 0; haid < 6; haid++) {
        for (int pmtid = 0; pmtid < 6; pmtid++) {
            for (int px_x = 0; px_x < 8; px_x++) {
                for (int px_y = 0; px_y < 8; px_y++) {
                    name = "scinti_bottom_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));

                    mat_name = "GSO";

                    G4VSolid *pBoxSolid =
                            new G4Box(name,
                                      SCINTI_PIX_WIDTH / 2.,
                                      SCINTI_PIX_WIDTH / 2.,
                                      BOTTOM_SCINTI_DEPTH / 2.);


                    G4LogicalVolume *pBoxLog =
                            new G4LogicalVolume(pBoxSolid,
                                                material->GetMaterialByName(&mat_name),
                                                name.c_str());


                    TVector3 pos_gso_root = phpos->GetPixPositionByID(haid, pmtid, px_x, px_y);


                    G4ThreeVector pos_gso(
                            pos_gso_root.X(),

                            pos_gso_root.Y(),

                            pos_gso_root.Z()
                    );

                   // cout << pos_gso_root.X() <<", "<< pos_gso_root.Y() <<", "<< pos_gso_root.Z() << endl;


                    G4VPhysicalVolume *aBoxPhys =
                            new G4PVPlacement(0,
                                              pos_gso,
                                              pBoxLog,
                                              name.c_str(),
                                              tpc_gas_volume
                                                      .get_logicvol("TPC_gas"),
                                              false,
                                              0);

                    physPSA.push_back(aBoxPhys);
                    PSATrans.push_back(new G4Translate3D(pos_gso));
                    //----------------------------------------------------------

                    name = "esr_bottom_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));

                    mat_name = "ESR";

                    pBoxSolid = new G4Box(name,
                                          SCINTI_PIX_WIDTH / 2.,
                                          SCINTI_PIX_WIDTH / 2.,
                                          ESR_DEPTH / 2.);

                    pBoxLog = new G4LogicalVolume(pBoxSolid,
                                                  material->GetMaterialByName(&mat_name),
                                                  name.c_str());


                    G4ThreeVector pos_esr = pos_gso
                                            + G4ThreeVector(0,
                                                            0,
                                                            -ESR_DEPTH / 2. - BOTTOM_SCINTI_DEPTH / 2);


                    aBoxPhys =
                            new G4PVPlacement(0,
                                              pos_esr,
                                              pBoxLog,
                                              name.c_str(),
                                              tpc_gas_volume
                                                      .get_logicvol("TPC_gas"),
                                              false,
                                              0);

                    //----------------------------------------------------------

                    name = "tefron_bottom_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));

                    mat_name = "teflon";

                    pBoxSolid = new G4Box(name,
                                          SCINTI_PIX_WIDTH / 2.,
                                          SCINTI_PIX_WIDTH / 2.,
                                          TEFLON_DEPTH / 2.);

                    pBoxLog = new G4LogicalVolume(pBoxSolid,
                                                  material->GetMaterialByName(&mat_name),
                                                  name.c_str());


                    G4ThreeVector pos_teflon =
                            pos_esr + G4ThreeVector(0, 0, -ESR_DEPTH / 2. - TEFLON_DEPTH / 2);

                    aBoxPhys =
                            new G4PVPlacement(0,
                                              pos_teflon,
                                              pBoxLog,
                                              name.c_str(),
                                              tpc_gas_volume
                                                      .get_logicvol("TPC_gas"),
                                              false,
                                              0);


                    //----------------------------------------------------------

                    name = "black_tape_bottom_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));


                    mat_name = "PVC";


                    pBoxSolid = new G4Box(name,
                                          SCINTI_PIX_WIDTH / 2.,
                                          SCINTI_PIX_WIDTH / 2.,
                                          TAPE_DEPTH / 2.);


                    pBoxLog = new G4LogicalVolume(pBoxSolid,
                                                  material->GetMaterialByName(&mat_name),
                                                  name.c_str());

                    G4ThreeVector pos_tape = pos_teflon +
                                             G4ThreeVector(0, 0, -TAPE_DEPTH / 2. - TEFLON_DEPTH / 2.);


                    aBoxPhys = new G4PVPlacement(0,
                                                 pos_tape,
                                                 pBoxLog,
                                                 name.c_str(),
                                                 tpc_gas_volume
                                                         .get_logicvol("TPC_gas"),
                                                 false,
                                                 0);

                }
            }
        }
    }

    //-----side-----


    double BASE_X = 181.6 * mm; //scinti-pmt dir
    const double BASE_Y = -178.75 * mm; //right corner
    //const double BASE_Z = 150.25 * mm + 22. * mm;
    //const double BASE_Z = 476. * mm - 328.75 * mm  + 3.*mm + 22. * mm;
    const double BASE_Z = 476. * mm - 328.75 * mm + 3. * mm;

    const double SIDE_SCINTI_DEPTH = 13. * mm;


    ofstream geo_chk("geo_chk.txt");

    //.qint cpnum = 0;

    geo_chk << "haid, pmtid, gso_x, gso_y, gso_z, esr_x, esr_y, esr_z" << endl;

    for (int haid = 6; haid < 18; haid++) {
        TVector3 haface = -phpos->GetFaceDirByID(haid);
        G4ThreeVector hafaceg4(haface.X(), haface.Y(), haface.Z());
        TVector3 hax = phpos->GetXDirByID(haid);
        TVector3 hay = phpos->GetYDirByID(haid);
        TVector3 geosize;

        for (int pmtid = 0; pmtid < 6; pmtid++) {
            for (int px_x = 0; px_x < 8; px_x++) {
                for (int px_y = 0; px_y < 8; px_y++) {
                    name = "scinti_side_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));


                    mat_name = "GSO";


                    geosize = (SIDE_SCINTI_DEPTH / 2.) * haface
                              + (SCINTI_PIX_WIDTH / 2.) * hax
                              + (SCINTI_PIX_WIDTH / 2.) * hay;


                    // cout << geosize.X() << ", " << geosize.Y() << ", " << geosize.Z() << endl;

                    G4VSolid *pBoxSolid =
                            new G4Box(name,
                                      fabs(geosize.X()),
                                      fabs(geosize.Y()),
                                      fabs(geosize.Z()));


                    G4LogicalVolume *pBoxLog =
                            new G4LogicalVolume(pBoxSolid,
                                                material->GetMaterialByName(&mat_name),
                                                name.c_str());

                    TVector3 pos_gso_root = phpos->GetPixPositionByID(haid, pmtid, px_x, px_y);

                    G4ThreeVector pos_gso(
                            pos_gso_root.X(),

                            pos_gso_root.Y(),

                            pos_gso_root.Z()
                    );


                    G4VPhysicalVolume *aBoxPhys =
                            new G4PVPlacement(0,
                                              pos_gso,
                                              pBoxLog,
                                              name.c_str(),
                                              tpc_gas_volume
                                                      .get_logicvol("TPC_gas"),
                                              false,
                                              0);

                    physPSA.push_back(aBoxPhys);
                    PSATrans.push_back(new G4Translate3D(pos_gso));

                    //----------------------------------------------------------

                    name = "esr_side_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));

                    mat_name = "ESR";

                    geosize = (ESR_DEPTH / 2.) * haface
                              + (SCINTI_PIX_WIDTH / 2.) * hax
                              + (SCINTI_PIX_WIDTH / 2.) * hay;


                    pBoxSolid = new G4Box(name,
                                          fabs(geosize.X()),
                                          fabs(geosize.Y()),
                                          fabs(geosize.Z()));

                    pBoxLog = new G4LogicalVolume(pBoxSolid,
                                                  material->GetMaterialByName(&mat_name),
                                                  name.c_str());


                    G4ThreeVector pos_esr = pos_gso
                                            - (ESR_DEPTH / 2. + SIDE_SCINTI_DEPTH / 2.) * hafaceg4;

                    aBoxPhys =
                            new G4PVPlacement(0,
                                              pos_esr,
                                              pBoxLog,
                                              name.c_str(),
                                              tpc_gas_volume
                                                      .get_logicvol("TPC_gas"),
                                              false,
                                              0);


                    //----------------------------------------------------------

                    name = "tefron_side_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));

                    mat_name = "teflon";

                    geosize = (TEFLON_DEPTH / 2.) * haface
                              + (SCINTI_PIX_WIDTH / 2.) * hax
                              + (SCINTI_PIX_WIDTH / 2.) * hay;
                    pBoxSolid = new G4Box(name,
                                          fabs(geosize.X()),
                                          fabs(geosize.Y()),
                                          fabs(geosize.Z()));


                    pBoxLog = new G4LogicalVolume(pBoxSolid,
                                                  material->GetMaterialByName(&mat_name),
                                                  name.c_str());


                    G4ThreeVector pos_teflon
                            = pos_esr - (ESR_DEPTH / 2. + TEFLON_DEPTH / 2.) * hafaceg4;


                    aBoxPhys =
                            new G4PVPlacement(0,
                                              pos_teflon,
                                              pBoxLog,
                                              name.c_str(),
                                              tpc_gas_volume
                                                      .get_logicvol("TPC_gas"),
                                              false,
                                              0);


                    //----------------------------------------------------------

                    name = "black_tape_side_"
                           + to_string(Get_HA_ID(haid,ID_TO_NAME)) + "_"
                           + to_string(Get_PMT_ID(pmtid,ID_TO_NAME)) + "_"
                           + to_string(Get_Pix_ID(pmtid,px_x+8*px_y,ID_TO_NAME));


                    mat_name = "PVC";

                    geosize = (TAPE_DEPTH / 2.) * haface
                              + (SCINTI_PIX_WIDTH / 2.) * hax
                              + (SCINTI_PIX_WIDTH / 2.) * hay;

                    pBoxSolid = new G4Box(name,
                                          fabs(geosize.X()),
                                          fabs(geosize.Y()),
                                          fabs(geosize.Z()));


                    pBoxLog = new G4LogicalVolume(pBoxSolid,
                                                  material->GetMaterialByName(&mat_name),
                                                  name.c_str());

                    G4ThreeVector pos_tape
                            = pos_teflon - (TAPE_DEPTH / 2. + TEFLON_DEPTH / 2.) * hafaceg4;


                    aBoxPhys =
                            new G4PVPlacement(0,
                                              pos_tape,
                                              pBoxLog,
                                              name.c_str(),
                                              tpc_gas_volume
                                                      .get_logicvol("TPC_gas"),
                                              false,
                                              0);


                    geo_chk << haid << ", " << pmtid << ", "
                    << pos_gso.getX() << ", "<< pos_gso.getY() << ", " << pos_gso.getZ() << ", "
                    << pos_esr.getX() << ", " <<pos_esr.getY() << ", " << pos_esr.getZ() << endl;
                }
            }
        }
    }

    geo_chk.close();

    if (geo_flag == GEO_FLAG_GROUND) {
        geo_data_name = data_dir + "/etcc_base_no_battery.mqo";
        //geo_data_name = data_dir + "/etcc_base.mqo";
		} else {
        geo_data_name = data_dir + "/etcc_base.mqo";
		}
    GGG_Model base_structure(geo_data_name);
    base_structure.
            create(material, vessel_inside
            .get_logicvol("Air"));

    geo_data_name = data_dir + "/etcc_inside.mqo";
    GGG_Model inside_structure;
    inside_structure.set_convert(-3, 1, -2);
    inside_structure.load(geo_data_name);


    inside_structure.set_object_visible("scinti_side", false);
    inside_structure.set_object_visible("scinti_bottom", false);

    //inside_structure.
    inside_structure.
            create(material,
                   tpc_gas_volume
                           .get_logicvol("TPC_gas"),
                   GGG_MODEL_DIVIDE);

    G4double maxStep = 40. * micrometer;
//geo_data_name = data_dir + "/detectable_area_forExp_simple.mqo";
//GGG_Model tpc_detectable(geo_data_name);
//tpc_detectable.create(material,
//                        tpc_gas_volume.get_logicvol("TPC_gas"),
//                        GGG_MODEL_DIVIDE);
//G4LogicalVolume *logicTPC = tpc_detectable.get_logicvol("TPC_detectable_2hit");
    logicTPC = inside_structure.get_logicvol("TPC_detectable");

    logicTPC->SetUserLimits(new G4UserLimits(maxStep));


    ofstream WeightFile("SMILE_2p.weight", ios::out);
    WeightFile << "/////////////////////////////////" << endl;
    WeightFile << " Total Mass       : "
               << logic_world->GetMass() / kg << endl;


    if (geo_flag == GEO_FLAG_FLIGHT) {
        WeightFile << "/////////////////////////////////" << endl;
        WeightFile << " Pressured Vessel : "
                   << pressured_vessel.get_logicvol("bCALET")->GetMass() / kg << endl;
    }

    WeightFile << "/////////////////////////////////" << endl;
    if (geo_flag != GEO_FLAG_NO_VESSEL) {
        WeightFile << " ETCC vessel  : "
                   << tpc_vessel.get_mass() << endl;
    }

    WeightFile << "/////////////////////////////////" << endl;
    WeightFile << " ETCC inside  : "
               << tpc_gas_volume.get_mass() << endl;


    WeightFile << "/////////////////////////////////" << endl;
    WeightFile << " Lower Circuits   : "
               << base_structure.get_mass()
               << endl;

    if (geo_flag == GEO_FLAG_FLIGHT) {
        WeightFile << "/////////////////////////////////" << endl;
        WeightFile << " Anti   : "
                   << etcc_anti.get_mass()
                   << endl;


        WeightFile << "/////////////////////////////////" << endl;
        WeightFile << " Gondola  : " << gondola.get_mass() << endl;


        WeightFile << "/////////////////////////////////" << endl;
        WeightFile << " blst : "
                   << (pBlstLog[0]->GetMass() +
                       pBlstLog[1]->GetMass() +
                       pBlstLog[2]->GetMass() +
                       pBlstLog[3]->GetMass()
                      ) / kg << endl;
    }

    cout << "geometry generation complete" << endl;
    return Phys_World;
}
