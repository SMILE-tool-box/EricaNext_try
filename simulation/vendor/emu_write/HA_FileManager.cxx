//
// Created by ynakamura_local on 10/12/18.
//

#include "HA_FileManager.h"
#include <iomanip>
#include <sstream>
#include <iostream>

using namespace std;


int HA_FileManager::increment_size = 1024 * 3;

string HA_FileManager::get_fname() {
    stringstream ss;
    ss << data_dir << "/HA"
       << setw(2) << setfill('0') << ha_name << "_"
       << setw(5) << setfill('0') << fcount << ".raw";

    return ss.str();
}


void HA_FileManager::increment() {
    fst.close();
    fcount++;
    create();
}

int HA_FileManager::get_fsize() {
    return fst.tellp() / 1e3;
}



//-----write-----

void HA_FileManager::write_header() {


    const int HEAD_SIZE = 8;
    char head_data[HEAD_SIZE];


    head_data[0] = 0xeb;
    head_data[1] = 0x90;
    head_data[2] = ((pckt_id >> 8) & 0xff);
    head_data[3] = (pckt_id & 0xff);
    head_data[4] = 0x00;
    head_data[5] = ((evdata.evid >> 16) & 0xff);
    head_data[6] = ((evdata.evid >> 8) & 0xff);
    head_data[7] = ((evdata.evid >> 0) & 0xff);

    fst.write(head_data, sizeof(char) * HEAD_SIZE);

    if(pckt_id +1 <0)pckt_id=0; //for overflow
    else pckt_id++;
}


void HA_FileManager::write_data() {
    const int MAIN_SIZE = 48;
    char ha_data[MAIN_SIZE] = {};
    for (int ch = 0; ch < 24; ch++) {
        ha_data[2 * ch] = ((evdata.adc[ch] >> 8) & 0xff);
        ha_data[2 * ch + 1] = ((evdata.adc[ch] >> 0) & 0xff);
    }
    fst.write(ha_data, sizeof(char) * MAIN_SIZE);
}

void HA_FileManager::create() {
    fst.open(get_fname().c_str(), ios::binary | ios::trunc);
    if (!fst) {
        cerr << "can't create file: " << get_fname() << endl;
        return;
    } else {
        cout << get_fname() << " was created." << endl;
    }

}

//-----public-----


HA_FileManager::HA_FileManager(const char *data_dir, int ha_name) {
    fcount = 0;
    pckt_id = 0;
    this->data_dir = data_dir;
    this->ha_name = ha_name;

    create();
}

HA_FileManager::~HA_FileManager() {
    fst.close();
}


void HA_FileManager::write_event(unsigned evid, unsigned short *adc) {
    evdata.evid = evid;
    for (int i = 0; i < 4 * 6; i++) {
        evdata.adc[i] = adc[i];
    }

    write_header();
    write_data();


    if (get_fsize() > increment_size) {
        increment();
    }

}