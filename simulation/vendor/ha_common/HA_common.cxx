#include "HA_common.h"

/*!
 * @file HA_common.cxx
 */

#include <TBox.h>
#include <iostream>
#include <iomanip>
#include <boost/bimap/bimap.hpp>


using namespace std;

constexpr int HA_NUM = 18;
constexpr int PMT_NUM = 6;
constexpr int CH_NUM = 4;
constexpr int PIX_NUM = 8 * 8;

//-----old-----
/*constexpr double geo[5][6] = {
        0, 0, 338.5, 174, 171, 13.,
        195.4, 15, 274, 6.5, 171, 87,
        -15, 195.4, 274, 171, 6.5, 87,
        -195.4, -15, 274, 6.5, 171, 87,
        15, -195.4, 274, 171, 6.5, 87
};
*/

/*!
 * @brief HA position and size(half) for the QL geometry
 */
constexpr double geo[5][6] = {
        0, 0, 354.1, 166.75, 166.75, 13.,
        181.6, 15, 227, 6.5, 171, 79.75,
        -15, 181.6, 227, 171, 6.5, 79.75,
        -181.6, -15, 227, 6.5, 171, 79.75,
        15, -181.6, 227, 171, 6.5, 79.75
};


typedef boost::bimaps::bimap<int, int> ha_map_t;  //ID to Name
typedef ha_map_t::value_type ha_map_value_t;
ha_map_t ha_map;

typedef boost::bimaps::bimap<int, int> pmt_map_t;  //ID to Name
typedef pmt_map_t::value_type pmt_map_value_t;
pmt_map_t pmt_map;


typedef boost::bimaps::bimap<int, HA_common::HA_PMT_CH> hpc_map_t;  //ID to Name
typedef hpc_map_t::value_type hpc_map_value_t;
hpc_map_t hpc_map;


bool IsInBox(double px, double py, double pz,
             double x, double y, double z, double dx, double dy, double dz) {
    bool sol;
    sol = (x - dx < px && px < x + dx) &&
          (y - dy < py && py < y + dy) &&
          (z - dz < pz && pz < z + dz);

    return sol;
}




//-----Init-----

void Set_HPC_Map() {
    HA_common::HA_PMT_CH hpc;
    int small_id;

    for (int id = 0; id < CH_NUM * PMT_NUM * HA_NUM; id++) {
        small_id = id % (CH_NUM * PMT_NUM);
        hpc.ha = HA_common::Get_HA_ID(id / (CH_NUM * PMT_NUM), HA_common::ID_TO_NAME);
        hpc.pmt = small_id % 3 + 1 + 3 * (small_id / 12);
        hpc.ch = small_id % 12 / 3 + 1;

        hpc_map.insert(hpc_map_value_t(id, hpc));
    }
}

void Set_HA_Map() {
    for (int i = 0; i < HA_NUM; i++) {
        ha_map.insert(ha_map_value_t(i, i + i / 3 * 3 + 1));
    }
}

void Set_PMT_Map() {
    pmt_map.insert(pmt_map_value_t(0, 6));
    pmt_map.insert(pmt_map_value_t(1, 1));
    pmt_map.insert(pmt_map_value_t(2, 3));
    pmt_map.insert(pmt_map_value_t(3, 4));
    pmt_map.insert(pmt_map_value_t(4, 5));
    pmt_map.insert(pmt_map_value_t(5, 2));
}

bool initialized = false;

void Chk_Initialized() {

    if (initialized == false) {
        initialized = true;
        Set_HA_Map();
        Set_PMT_Map();
        Set_HPC_Map();
        cout << "HA_common initialized" << endl;
    }
}


//-----IDs-----


//try
//{
//bm.left.at(5);
//}
//catch( const std::out_of_range &e )
//{
//std::cout << "Out of range exception occured. CAUSE = " << e.what() << std::endl;
//}

int HA_common::Get_HA_ID(int id_or_name, int lookuptype) {
    Chk_Initialized();
    int sol;
    if (lookuptype == HA_common::NAME_TO_ID) {
        return ha_map.right.at(id_or_name);
    } else if (lookuptype == HA_common::ID_TO_NAME) {
        return ha_map.left.at(id_or_name);
    }
    return -1;
}

int HA_common::Get_PMT_ID(int id_or_name, int lookuptype) {
    Chk_Initialized();

    if (lookuptype == HA_common::NAME_TO_ID) {
        return pmt_map.right.at(id_or_name);
    } else if (lookuptype == HA_common::ID_TO_NAME) {
        return pmt_map.left.at(id_or_name);
    }

    return -1;
}

int HA_common::Get_Pix_ID(int pmt, int pix, int lookuptype) {
    if (lookuptype == HA_common::NAME_TO_ID) {
        switch (pmt) {
            case 3:
            case 5:
            case 6:
                return 63 - pix;
            default :
                return pix;
        }
    } else if (lookuptype == HA_common::ID_TO_NAME) {
        switch (pmt) {
            case 1:
            case 3:
            case 5:
                return 63 - pix;
            default :
                return pix;
        }
    }
}


int HA_common::Get_CH_ID(int pmt, int ch, int lookuptype) {
    if (lookuptype == HA_common::NAME_TO_ID) {
        if (pmt == 5 || pmt == 3 || pmt == 6) {
            return ch - 1;
        } else {
            switch (ch) {
                case 1:
                    return 2;
                case 2:
                    return 3;
                case 3:
                    return 0;
                case 4:
                    return 1;
            }
        }
    } else if (lookuptype == HA_common::ID_TO_NAME) {
        if (pmt % 2 == 0)
            return ch + 1;
        else {
            switch (ch) {
                case 0:
                    return 3;
                case 1:
                    return 4;
                case 2:
                    return 1;
                case 3:
                    return 2;
            }
        }

    }

    return -1;
}

int HA_common::Get_HPC_ID(const HA_common::HA_PMT_CH &hpc, int lookuptype) {
    Chk_Initialized();

    if (lookuptype == HA_common::NAME_TO_ID) {
        return hpc_map.right.at(hpc);
    } else if (lookuptype == HA_common::ID_TO_ID) {


//-----
    }
    return -1;

}

HA_common::HA_PMT_CH HA_common::Get_HPC_ID(int id, int lookuptype) {
    Chk_Initialized();

    if (lookuptype == HA_common::ID_TO_NAME) {
        return hpc_map.left.at(id);
    } else if (lookuptype == HA_common::ID_TO_ID) {
        HA_common::HA_PMT_CH buf;
        buf = hpc_map.left.at(id);
        return HA_common::HA_PMT_CH(
                HA_common::Get_HA_ID(buf.ha, HA_common::NAME_TO_ID),
                HA_common::Get_PMT_ID(buf.pmt, HA_common::NAME_TO_ID),
                HA_common::Get_CH_ID(buf.pmt, buf.ch, HA_common::NAME_TO_ID)
        );
    }
    return HA_common::HA_PMT_CH(-1, -1, -1);

}


int HA_common::Get_HPPix_ID(int ha, int pmt, int pix, int lookuptype) {
    int sol = 0;
    if (lookuptype == HA_common::NAME_TO_ID) {
        sol += HA_common::Get_HA_ID(ha, HA_common::NAME_TO_ID) * PMT_NUM * PIX_NUM;
        sol += HA_common::Get_PMT_ID(pmt, HA_common::NAME_TO_ID) * PIX_NUM;
        switch (pmt) {
            case 3:
            case 5:
            case 6:
                sol += pix - 1;
                break;
            case 1:
            case 2:
            case 4:
                sol += (64 - pix);
                break;
            default:
                sol = -1;
        }
        return sol;
    } else if (lookuptype == HA_common::ID_TO_ID) {
        sol += ha * PMT_NUM * PIX_NUM;
        sol += pmt * PIX_NUM;
        sol += pix;
        return sol;
    }
    return -1;
}




//-----Prints-----

void HA_common::Print_HPC_ID() {
    for (int id = 0; id < HA_NUM * PMT_NUM * CH_NUM; id++) {
        HA_common::HA_PMT_CH hpc = HA_common::Get_HPC_ID(id, HA_common::ID_TO_NAME);
        cout << setw(2) << id
             << " <---> HA_" << setw(2) << hpc.ha
             << ", PMT_" << setw(2) << hpc.pmt
             << ", CH_" << setw(2) << hpc.ch << endl;
    }
}

void HA_common::Print_HA_ID() {
    cout << "test" << endl;
    for (int id = 0; id < HA_NUM; id++) {
        cout << setw(2) << id
             << " <---> HA_" << setw(2) << HA_common::Get_HA_ID(id, HA_common::ID_TO_NAME) << endl;
    }
}

void HA_common::Print_PMT_ID() {
    for (int id = 0; id < PMT_NUM; id++) {
        cout << setw(2) << id
             << " <---> PMT_" << setw(2) << HA_common::Get_PMT_ID(id, HA_common::ID_TO_NAME) << endl;
    }
}

void HA_common::Print_CH_ID() {
    for (int pmt = 0; pmt < PMT_NUM; pmt++) {
        for (int ch = 0; ch < CH_NUM; ch++) {
            cout << "PMT_ID:" << setw(2) << pmt << ", CH_ID:" << ch
                 << " <---> PMT_" << setw(2) << HA_common::Get_PMT_ID(pmt, HA_common::ID_TO_NAME)
                 << ", CH_" << setw(2) << HA_common::Get_CH_ID(pmt, ch, HA_common::ID_TO_NAME) << endl;
        }
    }
}


int HA_common::WhichFace(double x, double y, double z) {
    const double margin = 5;
    for (int i = 0; i < 5; i++) {
        if (IsInBox(x, y, z, geo[i][0], geo[i][1], geo[i][2], geo[i][3] + margin, geo[i][4] + margin,
                    geo[i][5] + margin)) {
            return i;
        }
    }
    return FACE_OUT;
}

TBox *CreateTBox(HA_common::FACE_ID faceid, int dir_x, int dir_y) {
    TBox *sol;

    sol = new TBox(geo[faceid][dir_x] - geo[faceid][dir_x + 3],
                   geo[faceid][dir_y] - geo[faceid][dir_y + 3],
                   geo[faceid][dir_x] + geo[faceid][dir_x + 3],
                   geo[faceid][dir_y] + geo[faceid][dir_y + 3]);

    sol->SetFillStyle(0);
    return sol;
}

void HA_common::DrawPMTBox(int xz_yz_xy) {
    static TBox *bottom[3];
    static TBox *side[3][4];
    static bool IsInit = false;

    if (!IsInit) {
        bottom[0] = CreateTBox(FACE_BOTTOM, 0, 2);

        bottom[1] = CreateTBox(FACE_BOTTOM, 1, 2);

        bottom[2] = CreateTBox(FACE_BOTTOM, 0, 1);

        side[0][0] = CreateTBox(FACE_EAST, 0, 2);
        side[0][1] = CreateTBox(FACE_WEST, 0, 2);

        side[1][0] = CreateTBox(FACE_SOUTH, 1, 2);
        side[1][1] = CreateTBox(FACE_NORTH, 1, 2);

        side[2][0] = CreateTBox(FACE_EAST, 0, 1);
        side[2][1] = CreateTBox(FACE_WEST, 0, 1);
        side[2][2] = CreateTBox(FACE_SOUTH, 0, 1);
        side[2][3] = CreateTBox(FACE_NORTH, 0, 1);

        IsInit = true;
    }

    bottom[xz_yz_xy]->Draw();

    int boxnum;
    switch (xz_yz_xy) {
        case 0:
        case 1:
            boxnum = 2;
            break;
        case 2:
            boxnum = 4;
    }
    for (int i = 0; i < boxnum; i++) {
        side[xz_yz_xy][i]->Draw();
    }
}


double HA_common::GetPMTMinZ() {
    return geo[1][2] - geo[1][2 + 3];
}
