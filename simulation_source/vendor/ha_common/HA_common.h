#ifndef HA_COMMON_LIBRARY_H
#define HA_COMMON_LIBRARY_H


/*!
 * @file HA_common.h
 * @brief Manage the HA ID and HA name
 * @details
 * Name: (circuit base) ref: 80256V2 how to use<br>
 * HA1,HA2,HA3,HA7,HA8,HA9....HA33<br>
 * PMT6,PMT1,PMT3,PMT4,PMT5,PMT2<br>
 * CH1,CH2,CH3,CH4<br>
 * <br>
 * ID: (spacial arrangement base)<br>
 * 0,1,2,3,4,5,...17<br>
 * 0,1,2,...5<br>
 * 0,1,2,3<br>
 * <br>
 * Set: (output data base)<br>
 * (HA1,PMT1,CH1),(HA1,PMT2,CH1),(HA1,PMT3,CH1),(HA1,PMT1,CH2),...()<br>
 * 0,1,2,...431<br>
 * @author Yuta Nakamura
 * @version 1.0
 * @date Ver 1.0 2018/xx/xx created
 */



#include <TVector3.h>

namespace HA_common {
    enum : int {
        NAME_TO_ID,
        ID_TO_NAME,
        ID_TO_ID,
        NAME_TO_NAME,
        NAME_TO_POSITION,
        ID_TO_POSITION
    };
    enum : int {
        PLACE_BOTTOM,
        PLACE_EAST,
        PLACE_SOUTH,
        PLACE_WEST,
        PLACE_NORTH
    };

    enum FACE_ID{
        FACE_BOTTOM,
        FACE_EAST,
        FACE_SOUTH,
        FACE_WEST,
        FACE_NORTH,
	FACE_OUT
    };

    



    struct HA_PMT_CH {
        int ha;
        int pmt;
        int ch;

        HA_PMT_CH() {
            ha = pmt = ch = -1;
        }

        HA_PMT_CH(int ha, int pmt, int ch) {
            this->ha = ha;
            this->pmt = pmt;
            this->ch = ch;
        }

        bool operator<(const HA_common::HA_PMT_CH a) const {
            bool sol;
            sol = ha < a.ha;
            sol = sol || (ha == a.ha && pmt < a.pmt);
            sol = sol || (ha == a.ha && pmt == a.pmt && ch < a.ch);
            return sol;
        }
    };

    struct HA_PMT_PIX {
        int ha;
        int pmt;
        int pix;

        HA_PMT_PIX() {
            ha = pmt = pix = -1;
        }
    };


    int Get_HA_ID(int id_or_name, int lookuptype = NAME_TO_ID);

    int Get_PMT_ID(int id_or_name, int lookuptype = NAME_TO_ID);

    int Get_Pix_ID(int pmt, int pix, int lookuptype = NAME_TO_ID);

    int Get_CH_ID(int pmt, int ch, int lookuptype = NAME_TO_ID);

    int Get_HPC_ID(const HA_PMT_CH &hpc, int lookuptype = NAME_TO_ID);

    HA_PMT_CH Get_HPC_ID(int id, int lookuptype = ID_TO_NAME);

    int Get_HPPix_ID(int ha, int pmt, int pix, int lookuptype = NAME_TO_ID);



    void Print_HA_ID();

    void Print_PMT_ID();

    void Print_CH_ID();

    void Print_HPC_ID();

    int WhichFace(double x, double y, double z);

    void DrawPMTBox(int xz_yz_xy);

    double GetPMTMinZ();
}





#endif
