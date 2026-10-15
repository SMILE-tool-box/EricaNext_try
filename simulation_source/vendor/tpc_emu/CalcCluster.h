#ifndef CalcCluster_h
#define CalcCluster_h

#include <string>
#include <vector>
#include "TF1.h"
#include "TH2F.h"
#include "TRandom3.h"

struct ClusterData {
    std::vector<int> size;
    std::vector<int> count;
    std::vector<double> rate;

private:
    int sum;

    void Update();
};

struct Cluster {
    double energy;
    ClusterData data;
};

class CalcCluster {
private:
    std::string fname;

    TRandom3 *rndm;
    TH2F *clust_data;
    TH2F *clust_data_n;

    void Load();

    void Update();

public:


    CalcCluster(std::string fname);
    TH2F* GetClustData(){return clust_data;}
    Int_t GetClustSize(Double_t edep);
    void SetSeed(UInt_t seed) { rndm->SetSeed(seed); }   // EricaSim: reproducible runs
};


#endif