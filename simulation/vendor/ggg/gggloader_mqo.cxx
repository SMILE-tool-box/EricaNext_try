//---------------------------------
//
//  class: GGG_Model
//  
//
//---------------------------------
#include "GGG_Model.h"
#include "gggloader.h"

#define GET_NEXT_WORD(delim)            buf_word = split(&(buf_line), delim)


using namespace GGG_LIB;

enum {
    CHUNK_SCENE,
    CHUNK_MATERIAL,
    CHUNK_OBJECT,
    CHUNK_DEFAULT
};

constexpr int MQO_VISIBLE = 15;
constexpr int MQO_NOT_VISIBLE = 0;

int GGG_Model::check_chunk(std::string *str) {
    std::string chunk = split(str, ' ');

    if (chunk == "Scene")return CHUNK_SCENE;
    else if (chunk == "Material")return CHUNK_MATERIAL;
    else if (chunk == "Object")return CHUNK_OBJECT;
    else return CHUNK_DEFAULT;
}

//get material name
void GGG_Model::load_material(std::string *p_buf_line) {
    std::string sol;
    std::string buf = split(p_buf_line, ' ');
    rmb_dblquot(&buf);
    for (unsigned i = 0; i < buf.size(); i++) {
        if (buf[i] == '[') {
            i++;
            while (i < buf.size() && buf[i] != ']') {
                sol += buf[i];
                i++;
            }
            break;
        }
    }

    material_list.push_back(buf);
}

void GGG_Model::load_object(std::ifstream *ifs, GGG_Object *p_obj) {
    unsigned mat_id = 0;
    std::string buf_line, buf_word;
    GGG_Solid solid_buf;

    while (1) {
        std::getline(*ifs, buf_line);
        GET_NEXT_WORD(' ');

        if (buf_word == "visible") {
            GET_NEXT_WORD(' ');
            switch (atoi(buf_word)) {
                case MQO_VISIBLE:
                    p_obj->set_visible(true);
                    break;
                case MQO_NOT_VISIBLE:
                    p_obj->set_visible(false);
                    break;
                default:
                    break;
            }
        } else if (buf_word == "vertex") {
            GET_NEXT_WORD(' ');
            int vnum = atoi(buf_word);
            for (int v_id = 0; v_id < vnum; v_id++) {
                double x[3], x_buf[3];
                std::getline(*ifs, buf_line);
                for (int i = 0; i < 3; i++) {
                    GET_NEXT_WORD(' ');
                    x[i] = atof(buf_word);
                }

                for(int i=0; i<3; i++){
                    x_buf[i] = sign(convert[i])*x[abs(convert[i])-1] + position_default[i];
                }
                for(int i=0; i<3; i++){
                    x[i] = x_buf[i];
                }


                G4ThreeVector vector(x[0], x[1], x[2]);
                solid_buf.add_vertex(vector);
            }
            std::getline(*ifs, buf_line);
        } else if (buf_word == "face") {
            GET_NEXT_WORD(' ');
            int fnum = atoi(buf_word);
            for (int f_id = 0; f_id < fnum; f_id++) {
                std::getline(*ifs, buf_line);
                GET_NEXT_WORD(' ');
                int vnum = atoi(buf_word);

                GET_NEXT_WORD('(');
                if (buf_word == "V") {
                    GGG_LIB::FACE face;
                    for (int i = 0; i < vnum; i++) {
                        if (i == vnum - 1)
                            GET_NEXT_WORD(')');
                        else
                            GET_NEXT_WORD(' ');
                        face.id.push_back(atoi(buf_word));
                    }
                    solid_buf.add_face(face);
                }

                GET_NEXT_WORD('(');
                if (buf_word == "M") {
                    GET_NEXT_WORD(')');
                    mat_id = atoi(buf_word);
                    if (mat_id >= material_list.size())
                        G4cerr << "material id is illegal (GGG)" << G4endl;
                }
            }
            solid_buf.set_name(p_obj->get_object_name());
            break;
        } else {}
    }
    p_obj->add_solid(solid_buf);
    p_obj->set_objmaterial_name(material_list[mat_id]);
}


void GGG_Model::load_mqo(std::ifstream *ifs) {
    std::string buf_line, buf_word;
    int chunk = CHUNK_DEFAULT;
    int depth = 0;
    int material_num = 0;
    int current_object_num = 0;

    while (std::getline(*ifs, buf_line)) {
        if (depth == 0) {
            chunk = check_chunk(&(buf_line));

            switch (chunk) {
                case CHUNK_DEFAULT:
                    break;

                case CHUNK_SCENE:
                    break;

                case CHUNK_MATERIAL:
                    material_num = atoi(split(&(buf_line), ' '));
                    depth++;
                    break;

                case CHUNK_OBJECT:

                    buf_word = split(&buf_line, ' ');
                    rmb_dblquot(&buf_word);
                    object_list.resize(object_list.size() + 1);
                    object_list[current_object_num].set_object_name(buf_word);
                    G4cout << "loading " << buf_word << G4endl;
                    depth++;
                    break;

                default:
                    break;
            }
        } else if (depth > 0) {
            switch (chunk) {
                case CHUNK_DEFAULT:
                    break;

                case CHUNK_SCENE:
                    break;

                case CHUNK_MATERIAL:
                    load_material(&buf_line);
                    if (material_num == 0)
                        depth--;
                    else
                        material_num--;
                    break;

                case CHUNK_OBJECT:
                    load_object(ifs, &(object_list[current_object_num]));
                    current_object_num++;
                    depth--;
                    break;

                default:
                    break;
            }
        } else {}
    }
}
