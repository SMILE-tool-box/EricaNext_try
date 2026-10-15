//
// Created by ynakamura_local on 10/12/18.
//

#ifndef ERICA_HA_FILEMANAGER_H
#define ERICA_HA_FILEMANAGER_H

#include <string>
#include <fstream>


struct HA_EventData {
    unsigned evid;
    unsigned short adc[4 * 6];
    int nhit;
};

class HA_FileManager {
    int ha_name;
    std::string data_dir;

    short pckt_id;

    std::ofstream fst;
    int fcount;

    HA_EventData evdata;

    static int increment_size;


    std::string get_fname();
    void write_header();
    void write_data();

    int get_fsize();

    void increment();

    void create();

public:
    HA_FileManager(const char *data_dir, int ha_name);

    ~HA_FileManager();

    void write_event(unsigned evid, unsigned short *adc);


};


#endif //ERICA_HA_FILEMANAGER_H
