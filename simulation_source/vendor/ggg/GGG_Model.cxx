#include "GGG_Model.h"

enum {
    FORMAT_MQO,
    FORMAT_OBJ,
    FORMAT_ERROR
};








//---------------------------------------------------
//
// static
//
//---------------------------------------------------
int GGG_Model::convert_default[3] = {1,2,3};
double GGG_Model::position_default[3] = {0,0,0};

void GGG_Model::set_convert_default(const int new_x, const int new_y, const int new_z){
    convert_default[0] = new_x;
    convert_default[1] = new_y;
    convert_default[2] = new_z;
}


void GGG_Model::set_position_default(const double x, const double y, const double z){
    position_default[0] = x;
    position_default[1] = y;
    position_default[2] = z;
}




//---------------------------------------------------
//
// construct and destruct
//
//---------------------------------------------------
GGG_Model::GGG_Model() {
    set_default_value();
}

GGG_Model::GGG_Model(std::string fname, bool divflag) {
    set_default_value();
    load(fname);
    if (divflag == GGG_MODEL_DIVIDE) {
        divide_solid();
    }
}

GGG_Model::GGG_Model(std::string fname, SMILE_Material *mat,
                     G4LogicalVolume *logic_world, bool divflag) {
    set_default_value();

    if (load(fname)) {
        create(mat, logic_world);
        if (divflag == GGG_MODEL_DIVIDE) {
            divide_solid();
        }
    }
}

GGG_Model::~GGG_Model() {
}


//---------------------------------------------------
//
// initialize
//
//---------------------------------------------------
bool GGG_Model::load(std::string fname) {
    G4cout << "loading " << fname << " (GGG)..." << G4endl;
    std::ifstream ifs(fname);
    if (ifs.fail()) {
        G4cerr << "ERROR: can't load " << fname << " (GGG)" << G4endl;
        return false;
    }

    switch (check_format(&ifs)) {
        case FORMAT_MQO:
            load_mqo(&ifs);
            break;
        default:
            G4cerr << "ERROR: " << fname << "'s format is not .mqo (GGG)" << G4endl;
            return false;
    }

    G4cout << "load end (GGG)" << G4endl
           << "object list:" << G4endl
           << "\t" << "object num: " << get_object_num() << G4endl;
    for (int i = 0; i < get_object_num(); i++) {
        G4cout << "\t\t" << "object id: " << i << G4endl
               << "\t\t\t" << "name: " << get_object_name(i) << G4endl
               << "\t\t\t" << "material name: "
               << get_objmaterial_name(i) << G4endl;
    }

    return true;
}

void GGG_Model::divide_solid() {
    for (unsigned i = 0; i < object_list.size(); i++)
        object_list[i].divide_solid();
}

void GGG_Model::create(SMILE_Material *mat,
                       G4LogicalVolume *logic_world,
                       G4bool checkOverlaps) {
    int obj_num = get_object_num();
    for (int i = 0; i < obj_num; i++) {
        if (object_list[i].get_visible() == true) {
            object_list[i].create(mat, logic_world, rotation,
                                  position, checkOverlaps);
        }else{
            G4cout << object_list[i].get_object_name()
                   << " was not created." << G4endl;
        }
    }
    G4cout << "--- all created ---" << G4endl;
}

//---------------------------------------------------
//
//  set variable
//
//---------------------------------------------------
void GGG_Model::set_unit(const G4double scale) {
    this->unit = scale;
}

void GGG_Model::set_rotation(G4RotationMatrix *const rot) {
    this->rotation = rot;
}

void GGG_Model::set_position(const G4ThreeVector &pos) {
    this->position = pos;
}

void GGG_Model::set_material(const int object_id,
                             const std::string material_name) {
    if (object_id < get_object_num()) {
        object_list[object_id].set_objmaterial_name(material_name);
    } else {
        // error
        G4cerr << "ERROR: can't set material (GGG)" << G4endl;
        return;
    }
    G4cout << "the object (id:" << object_id
           << ") material changed to " << material_name << " (GGG)" << G4endl;
}

void GGG_Model::set_material(const std::string object_name,
                             const std::string material_name) {
    int obj_num = get_object_num();
    for (int i = 0; i < obj_num; i++) {
        if (object_list[i].get_object_name() == object_name) {
            object_list[i].set_objmaterial_name(material_name);
            break;
        }
        if (i == obj_num - 1) {
            //error
            G4cerr << "ERROR: not found the object, "
                   << object_name << " (GGG)" << G4endl;
            return;
        }
    }
    G4cout << "the object (name:" << object_name
           << ") material changed to " << material_name << " (GGG)" << G4endl;
}

void GGG_Model::set_material_by_mat_name(const std::string old_material_name,
                                         const std::string new_material_name) {
    int obj_num = get_object_num();
    for (int i = 0; i < obj_num; i++) {
        if (object_list[i].get_objmaterial_name() == old_material_name) {
            object_list[i].set_objmaterial_name(new_material_name);
            break;
        }
        if (i == obj_num - 1) {
            //error
            G4cerr << "ERROR: not found the material, "
                   << old_material_name << " (GGG)" << G4endl;
            return;
        }
    }
    G4cout << "the material (name:" << old_material_name
           << ") is replaced with " << new_material_name << " (GGG)" << G4endl;
}

void GGG_Model::set_object_visible(const int object_id, const bool flag) {
    if (object_id < get_object_num()) {
        object_list[object_id].set_visible(flag);
    } else {
        //error
        G4cerr << "ERROR: object id is illigal (GGG)" << G4endl;
        return;
    }
}

void GGG_Model::set_object_visible(const std::string object_name, const bool flag) {
    int obj_num = get_object_num();
    for (int i = 0; i < obj_num; i++) {
        if (object_list[i].get_object_name() == object_name) {
            object_list[i].set_visible(flag);
            break;
        }
        if (i == obj_num - 1) {
            //error
            G4cerr << "ERROR: not found " << object_name << " (GGG)" << G4endl;
            return;
        }
    }

    if (flag) {
        G4cout << "the object (name:" << object_name << ") is set visible." << " (GGG)" << G4endl;
    } else {
        G4cout << "the object (name:" << object_name << ") is set invisible. " << " (GGG)" << G4endl;
    }
}


void GGG_Model::set_convert(const int new_x, const int new_y, const int new_z){
    convert[0] = new_x;
    convert[1] = new_y;
    convert[2] = new_z;
}

//---------------------------------------------------
//
//  get variable
//
//---------------------------------------------------
G4double GGG_Model::get_unit() {
    return unit;
}

G4double GGG_Model::get_mass(){
    int objnum = get_object_num();
    G4double weight = 0;
    for (int objid = 0; objid < objnum; objid++) {
        if(!object_list[objid].get_visible()) continue;

        int lognum = get_object_logicvol_num(objid);
        for (int logid = 0; logid < lognum; logid++) {
            weight += get_logicvol(objid, logid)->GetMass() / kg;
        }
    }
    return weight;
}

std::string GGG_Model::get_unit_by_symbol() {
    std::string symbol;

    std::map<double, std::string> mp_value2symbol;
    mp_value2symbol[nm] = "nm";
    mp_value2symbol[um] = "um";
    mp_value2symbol[mm] = "mm";
    mp_value2symbol[cm] = "cm";
    mp_value2symbol[m] = "m";
    mp_value2symbol[km] = "km";

    symbol = mp_value2symbol[unit];
    if (symbol == "") symbol = "unknown";

    return symbol;
}

int GGG_Model::get_object_num() {
    return object_list.size();
}

std::string GGG_Model::get_object_name(int object_num) {
    return object_list[object_num].get_object_name();
}

std::string GGG_Model::get_objmaterial_name(int object_num) {
    return object_list[object_num].get_objmaterial_name();
}

G4LogicalVolume *GGG_Model::get_logicvol(const std::string object_name,
                                         int id) {
    G4LogicalVolume *sol = nullptr;
    int obj_num = get_object_num();
    for (int i = 0; i < obj_num; i++) {
        if (object_list[i].get_object_name() == object_name) {
            sol = object_list[i].get_logicvol(id);
            break;
        }
        if (i == obj_num - 1) {
            //error
            G4cerr << "ERROR: not found the object, "
                   << object_name << " (GGG)" << G4endl;
        }
    }
    return sol;
}

G4LogicalVolume *GGG_Model::get_logicvol(int objid, int logicid){
    G4LogicalVolume *sol = nullptr;
    sol = object_list[objid].get_logicvol(logicid);
    return sol;
}



int GGG_Model::get_object_logicvol_num(const std::string object_name){
    int obj_num = get_object_num();
    for (int i = 0; i < obj_num; i++) {
        if (object_list[i].get_object_name() == object_name) {

            return object_list[i].get_solid_num();
        }
        if (i == obj_num - 1) {
            //error
            G4cerr << "ERROR: not found " << object_name << " (GGG)" << G4endl;
            return -1;
        }
    }
    return -1;
}

int GGG_Model::get_object_logicvol_num(const int object_id){
    if(object_id>=get_object_num()){
        G4cerr << "ERROR: object id is illigal (GGG)" << G4endl;
        return -1;
    }else{
        return object_list[object_id].get_solid_num();
    }
}




//---------------------------------------------------
//
//  private
//
//---------------------------------------------------

void GGG_Model::set_default_value() {
    unit = mm;
    rotation = nullptr;
    position = G4ThreeVector(0, 0, 0);

    for(int i=0; i<3; i++){
        convert[i] = convert_default[i];
    }

}

int GGG_Model::check_format(std::ifstream *ifs) {
    std::string buf;

    std::getline(*ifs, buf);
    if (buf == "Metasequoia Document") {
        ifs->seekg(0);
        return FORMAT_MQO;
    }

    return FORMAT_ERROR;
}


//---------------------------------------------------
//
//  developing
//
//---------------------------------------------------

//for obj file
//void GGG_Model::load_obj(std::ifstream *ifs) {
//}







