/*!
 * @file HAPositionManager.h
 * @brief Get the HA position from ID
 * @author Yuta Nakamura
 * @version 1.0
 * @date 2019/04/02 created
 */


#ifndef PROJECT_HAPOSITIONMANAGER_H
#define PROJECT_HAPOSITIONMANAGER_H

#include <fstream>
#include <string>
#include <vector>
#include <TVector3.h>

/*!
 * @brief Get the HA position from ID
 */
class HAPositionManager {
    struct ha_posdata {
        int name;
        double ha_c[3];
        int H_dir[2][3];
    };


    std::vector<ha_posdata> ha_pos;


public:
    int LoadPosData(std::string fname);
    TVector3 GetPixPositionByID(int ha_id, int pmt, int x_id, int y_id) ;
    TVector3 GetPixPositionByID(int ha_id, int pmt_id, int pix_id) ;
    TVector3 GetPixPositionByName(int ha_name, int pmt, int x_id, int y_id) ;
    TVector3 GetFaceDirByID(int ha_id);
    TVector3 GetFaceDirByName(int ha_name);

    TVector3 GetXDirByID(int ha_id);
    TVector3 GetXDirByName(int ha_name);

    TVector3 GetYDirByID(int ha_id);
    TVector3 GetYDirByName(int ha_name);
};


#endif //PROJECT_HAPOSITIONMANAGER_H
