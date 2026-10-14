//
// Created by ynakamura_local on 10/3/18.
//

#ifndef ERICA_TPC_FILEMANAGER_H
#define ERICA_TPC_FILEMANAGER_H

#include <string>
#include <fstream>
#include <vector>


struct TPC_EventData {
    unsigned evid;
    unsigned iwaki_time;
    unsigned short fadc[4][512];
    unsigned hit[1024 * 5]; // clk + 4 chips
    int nhit;
};

class TPC_FileManager {
    int fcount;
    std::ofstream fst;
    int fst_st, fst_now;

    std::string data_dir;
    static int increment_size; //[kB]

    int evcnt, clkcnt;

    int b_id;
    int ac;

    static int v_year;
    static int v_month;
    static int v_sub;
    static bool Reverse;

    int clk_depth;

    int enc_clk;

    TPC_EventData evdata;

    void write_version();

    void write_event_header();

    void write_fadc();

    void write_hit();

    void write_event_footer();

    std::string get_fname();


    int get_fsize(); //[kB]

    void increment();
    void create();

public:
    TPC_FileManager(const char *data_dir, int b_id, int ac);

    ~TPC_FileManager();



    void write_event(unsigned evid, unsigned time, unsigned *hit, unsigned short *fadc, int nhit);



    static void set_version(int year, int month, int sub) {
        v_year = year;
        v_month = month;
        v_sub = sub;
    }

    static void set_reverse(bool flag) {
        Reverse = flag;
    }


};

#endif //ERICA_TPC_FILEMANAGER_H
