//
// Created by ynakamura_local on 10/3/18.
//

#include "TPC_FileManager.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <boost/format.hpp>


using namespace std;


int TPC_FileManager::increment_size = 1024 * 3;

int TPC_FileManager::v_year = 2018;
int TPC_FileManager::v_month = 1;
int TPC_FileManager::v_sub = 0;
bool TPC_FileManager::Reverse = true;






void TPC_FileManager::create() {
    fst.open(get_fname().c_str(), ios::binary | ios::trunc);

    if (fst) {
        cout << get_fname() << " was created." << endl;
    } else {
        cerr << "can't create file: " << get_fname() << endl;
    }
}


void TPC_FileManager::increment() {
    fst.close();
    fcount++;
    create();
}

int TPC_FileManager::get_fsize() {
    return fst.tellp() / 1e3;
}


void TPC_FileManager::write_event_header() {
    const unsigned char  head[] = {0xeb, 0x90, 0x19, 0x64};
    unsigned char ta_data[16];

    for (int i = 0; i < 4; i++) {
        ta_data[i] = head[i];
    }

    ta_data[4] = 0x00;
    ta_data[5] = 0x00;
    ta_data[6] = 0x00;
    ta_data[7] = 0x01; // preliminary


    ta_data[8] = ((evdata.iwaki_time >> 24) & 0xff);
    ta_data[9] = ((evdata.iwaki_time >> 16) & 0xff);
    ta_data[10] = ((evdata.iwaki_time >> 8) & 0xff);
    ta_data[11] = ((evdata.iwaki_time >> 0) & 0xff);

    ta_data[12] = 0x00;
    ta_data[13] = ((evdata.evid >> 16) & 0xff);
    ta_data[14] = ((evdata.evid >> 8) & 0xff);
    ta_data[15] = ((evdata.evid >> 0) & 0xff);

    fst.write((char *)ta_data, sizeof(char) * 16);
}


void TPC_FileManager::write_fadc() {
    unsigned char w_tmp[2];

    for (int clock = 0; clock < 512; clock++) {
        for (int ch = 0; ch < 4; ch++) {
            if (Reverse) {
                int kr = 2 * (int) (ch / 2) + (1 - (ch % 2));
                w_tmp[0] = (((evdata.fadc[kr][clock] >> 8) & 0x03) | (((ch + 4) << 4) & 0xf0));
                w_tmp[1] = ((evdata.fadc[kr][clock] >> 0) & 0xff);
            } else {
                w_tmp[0] = (((evdata.fadc[ch][clock] >> 8) & 0x03) | (((ch + 4) << 4) & 0xf0));
                w_tmp[1] = ((evdata.fadc[ch][clock] >> 0) & 0xff);
            }

            fst.write((char *)w_tmp, sizeof(char) * 2);
        }
    }
}


void TPC_FileManager::write_version() {
    const unsigned char ta_data[] = {0x16, 0x03, 0x14, 0x00};

    fst.write((char *)ta_data, sizeof(char) * 4);
}


void TPC_FileManager::write_hit() {

    for (int clock = 0; clock < evdata.nhit; clock++) {


        unsigned char j_flag[4] = {0x80, 0x00, 0x00, 0x00};

        j_flag[2] = ((evdata.hit[5 * clock] >> 8) & 0x07);
        j_flag[3] = (evdata.hit[5 * clock] & 0xff);
        fst.write((char *)j_flag, 4);

        unsigned char hit_raw[16] = {};


        if (Reverse) {
            for (int k = 0; k < 4; k++) {
                int kr = 2 * (1 - (int) (k / 2)) + (k % 2);
                for (int ci = 0; ci < 4; ci++) {
                    for (int bit = 0; bit < 8; bit++) {
                        int rb = 7 - 2 * (int) (bit / 2) - (1 - (bit % 2));
                        char strip_data = ((evdata.hit[5 * clock + k + 1] >> (31 - 8 * ci - bit)) & 0x1);
                        hit_raw[4 * kr + ci] |= (strip_data << rb);
                    }
                }
            }
        } else {
            for (int k = 0; k < 4; k++) {
                for (int ci = 0; ci < 4; ci++) {
                    for (int bit = 0; bit < 8; bit++) {
                        char strip_data = ((evdata.hit[5 * clock + k + 1] >> (31 - 8 * ci - bit)) & 0x1);
                        hit_raw[4 * k + ci] |= (strip_data << (7 - bit));

                    }
                }
            }
        }
        fst.write((char *)hit_raw, sizeof(char) * 16);
    }

}

void TPC_FileManager::write_event_footer() {
    unsigned char foot[] = {0x75, 0x50, 0x49, 0x43};
    fst.write((char *)foot, sizeof(char) * 4);
}


string TPC_FileManager::get_fname() {
    stringstream ss;
    ss << data_dir << "/";

    if (ac == 0)
        ss << "uTPC_anode";
    else
        ss << "uTPC_cathode";

    ss << b_id << "_"
       << setw(5) << setfill('0') << fcount << ".raw";

    return ss.str();
}

//-----public-----


TPC_FileManager::TPC_FileManager(const char *data_dir, int b_id, int ac) {
    fcount = 0;
    this->data_dir = data_dir;
    this->b_id = b_id;
    this->ac = ac;

    create();
}

TPC_FileManager::~TPC_FileManager() {
    fst.close();
}



void TPC_FileManager::write_event(unsigned evid, unsigned time, unsigned *hit, unsigned short *fadc, int nhit) {
    evdata.evid = evid;
    evdata.iwaki_time = time;
    evdata.nhit = nhit;

    for (int i = 0; i < evdata.nhit * 5; i++) {
        evdata.hit[i] = hit[i];
    }
    for (int i = 0; i < 4 * 512; i++) {
        evdata.fadc[i / 512][i % 512] = fadc[i];
    }


    write_event_header();
    write_fadc();
    write_version();
    write_hit();
    write_event_footer();


    if (get_fsize() >= increment_size) {
        increment();
    }
}
