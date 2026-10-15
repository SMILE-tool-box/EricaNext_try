#ifndef GGG_MODEL_H_
#define GGG_MODEL_H_

#include "GGG_Object.h"
#include "SMILE_Material.h"

#define GGG_MODEL_DIVIDE        true
#define GGG_MODEL_NOT_DIVIDE    false


class GGG_Model {
    static int convert_default[3];
    static double position_default[3];


    G4double unit;    //default: mm
    std::vector<std::string> material_list;

    std::vector<GGG_Object> object_list;

    G4RotationMatrix *rotation;
    G4ThreeVector position;
    int convert[3];


    void set_default_value();

    //check file format. not change file pointer
    int check_format(std::ifstream *ifs);

    //---writen in gggloader_mqo.cpp---
    int check_chunk(std::string *str);

    void load_material(std::string *p_buf_line);

    void load_object(std::ifstream *ifs, GGG_Object *p_obj);

    //load function for .mqo file
    void load_mqo(std::ifstream *ifs);
    //---now developing---
    //load function for .obj file(now developing)
    //void load_obj(std::ifstream *ifs);




public:
    GGG_Model();

    GGG_Model(std::string fname, bool divflag = GGG_MODEL_NOT_DIVIDE);

    GGG_Model(std::string fname, SMILE_Material *nist,
              G4LogicalVolume *logic_world,
              bool divflag = GGG_MODEL_NOT_DIVIDE);

    ~GGG_Model();

    // init
    bool load(std::string fname);

    void divide_solid();

    void create(SMILE_Material *mat, G4LogicalVolume *logic_world,
                G4bool checkOverlaps = false);

    // set value
    void set_unit(const G4double unit);

    void set_rotation(G4RotationMatrix *const rotation);

    void set_position(const G4ThreeVector &posision);

    void set_material(const int object_id, const std::string material_name);

    void set_material(const std::string object_name,
                      const std::string material_name);

    void set_material_by_mat_name(const std::string old_material_name,
                                  const std::string new_material_name);

    void set_object_visible(const int object_id, const bool flag);

    void set_object_visible(const std::string object_name, const bool flag);

    void set_convert(const int new_x, const int new_y, const int new_z);
    static void set_convert_default(const int new_x, const int new_y, const int new_z);
    //model coordinate to Geant4 coordinate 1:x, 2:y, 3:z

    static void set_position_default(const double x, double y, double z);

    //template<typename G4VSD>
    //void set_sensitive_detector(const std::string object_name,
    //				const std::string sd_name,
    //				G4VSD** aValue, G4SDManager* SDmanager);

    // get value
    G4double get_unit();

    G4double get_mass();

    std::string get_unit_by_symbol();

    int get_object_num();

    int get_object_logicvol_num(const std::string object_name);

    int get_object_logicvol_num(const int object_id);

    std::string get_object_name(int object_num);

    std::string get_objmaterial_name(int object_num);

    G4LogicalVolume *get_logicvol(const std::string object_name, int id = 0);

    G4LogicalVolume *get_logicvol(int objid, int logicid = 0);
};


//template<typename G4VSD> void GGG_Model::set_sensitive_detector
//(const std::string object_name, const std::string sd_name,
//G4VSD** aValue, G4SDManager* SDmanager) {
//int obj_num = get_object_num();
//for (int i = 0; i < obj_num; i++) {
//  if (object_list[i].get_object_name() == object_name) {
//    object_list[i].set_sensitive_detector(sd_name, aValue, SDmanager);
//    break;
//  }
//  if (i == obj_num - 1) {
//    //error
//    G4cerr << "ERROR: not found the object, " << object_name 
//	     << " (GGG)" << G4endl;
//    return;
//  }
//}
//G4cout << "the object (name:" << object_name 
//	 << ") is set as sensiive detector (GGG)" << G4endl;
//}

#endif  // GGG_MODEL_H_
