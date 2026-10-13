/*!
 * @file HAPositionManager.cxx
 */

#include "HAPositionManager.h"
#include "HA_common.h"
#include "SM_Log.h"

using namespace std;
using namespace smlib;

constexpr int HA_NUM = 18;

using namespace HA_common;


int HAPositionManager::LoadPosData(std::string fname) {
    std::ifstream fpos;

    fpos.open(fname.c_str(), ios::in);
    if (!fpos) {
        SM_LOGGER(error) << "  Cannot find " << fname;
        return -1;
    }

    ha_pos.resize(HA_NUM);
    for (int i = 0; i < HA_NUM; i++) {
        fpos >> ha_pos[i].name
             >> ha_pos[i].ha_c[0] >> ha_pos[i].ha_c[1] >> ha_pos[i].ha_c[2]
             >> ha_pos[i].H_dir[0][0] >> ha_pos[i].H_dir[1][0]
             >> ha_pos[i].H_dir[0][1] >> ha_pos[i].H_dir[1][1]
             >> ha_pos[i].H_dir[0][2] >> ha_pos[i].H_dir[1][2];
    }
    fpos.close();


    return 0;
}


TVector3 HAPositionManager::GetPixPositionByID(int ha_id, int pmt_id, int x_id, int y_id) {
    double ha[2];
    double pos[3];

    ha[0] = 54. * ((int) (pmt_id % 2) - 0.5) + 6. * (x_id - 3.5);
    ha[1] = 54. * (1 - (int) (pmt_id / 2)) + 6. * (y_id - 3.5);


    for (int xyz = 0; xyz < 3; xyz++) {
        pos[xyz] = ha_pos[ha_id].ha_c[xyz];
        for (int XY = 0; XY < 2; XY++) {
            pos[xyz] += ha_pos[ha_id].H_dir[XY][xyz] * ha[XY];
        }
    }

    return TVector3(pos[0], pos[1], pos[2]);
}

TVector3 HAPositionManager::GetPixPositionByID(int ha_id, int pmt_id, int pix_id) {
    int x_id, y_id;
    x_id = pix_id % 8;
    y_id = pix_id / 8;

    return GetPixPositionByID(ha_id, pmt_id, x_id, y_id);
}


TVector3 HAPositionManager::GetPixPositionByName(int ha_name, int pmt_name, int x_id, int y_id) {
    int ha_id = Get_HA_ID(ha_name, NAME_TO_ID);
    int pmt_id = Get_PMT_ID(pmt_name, NAME_TO_ID);
    return GetPixPositionByID(ha_id, pmt_id, x_id, y_id);
}


TVector3 HAPositionManager::GetFaceDirByID(int ha_id) {
    TVector3 tmp[2];
    for (int i = 0; i < 2; i++) {
        tmp[i].SetXYZ(ha_pos[ha_id].H_dir[i][0],
                      ha_pos[ha_id].H_dir[i][1],
                      ha_pos[ha_id].H_dir[i][2]);
    }
    return tmp[0].Cross(tmp[1]);
}

TVector3 HAPositionManager::GetFaceDirByName(int ha_name) {
    int ha_id = Get_HA_ID(ha_name, NAME_TO_ID);
    return GetFaceDirByID(ha_id);
}

TVector3 HAPositionManager::GetXDirByID(int ha_id) {
    return TVector3(ha_pos[ha_id].H_dir[0][0],
                    ha_pos[ha_id].H_dir[0][1],
                    ha_pos[ha_id].H_dir[0][2]);
}

TVector3 HAPositionManager::GetXDirByName(int ha_name) {
    int ha_id = Get_HA_ID(ha_name, NAME_TO_ID);
    return GetXDirByID(ha_id);
}

TVector3 HAPositionManager::GetYDirByID(int ha_id) {
    return TVector3(ha_pos[ha_id].H_dir[1][0],
                    ha_pos[ha_id].H_dir[1][1],
                    ha_pos[ha_id].H_dir[1][2]);
}

TVector3 HAPositionManager::GetYDirByName(int ha_name) {
    int ha_id = Get_HA_ID(ha_name, NAME_TO_ID);
    return GetYDirByID(ha_id);
}







