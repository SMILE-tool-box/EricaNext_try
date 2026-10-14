#include "CalcCluster.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <TMath.h>


using namespace std;

const int CLUST_SIZE_MAX = 50;
const int ENERGY_MIN = 14;
const int ENERGY_MAX = 500;
const int COUNT_NUM = 1e4;

//cluster parameters
double peak_a = 0.04116;
double peak_b = 0.5336;
double sigma_a = 9.56e-2;
double sigma_b = 0.462;

Int_t CalcCluster::GetClustSize(Double_t edep) {
    Double_t sol = -1;
    double r;

    if (edep >= ENERGY_MAX) {
        sol = rndm->Gaus(peak_b + edep * peak_a, sigma_a * pow(edep, sigma_b));

        if (sol < 0) {
            //cerr << "ERROR(CalcCluster, high energy range)" << endl;
            sol = 0;
        }
        return sol;

    }

    while (sol < 0) {
        r = rndm->Uniform(2.*COUNT_NUM);

        for (int size = 0; size < clust_data_n->GetYaxis()->GetNbins(); size++) {

            if ((clust_data_n->GetBinContent(TMath::Floor(edep)+1, size+1)
                 + clust_data_n->GetBinContent(TMath::Floor(edep) + 2, size+1)) >= r) {
                sol = clust_data_n->GetYaxis()->GetBinCenter(size+1);
                break;
            }
        }

        if (sol == -1) {
            cerr << "ERROR(CalcCluster, low energy range)" << endl;
            cerr << "electron energy:" << edep << "[eV], r:" << r << endl;
            cerr << "rate:" << clust_data_n->GetBinContent(TMath::Floor(edep), 40) << endl;
            sol--;
        }
    }


    return sol;
}


CalcCluster::CalcCluster(string fname) {
    clust_data = new TH2F("ClusterSize", "ClusterSize;energy[eV];size",
                          ENERGY_MAX + 1, -0.5, ENERGY_MAX + 0.5,
                          CLUST_SIZE_MAX, 0.5, CLUST_SIZE_MAX + 0.5);
    clust_data->SetDirectory(nullptr);   // EricaSim: keep it out of open output files
    clust_data_n = nullptr;
    rndm = new TRandom3;
    this->fname = fname;
    Load();
}


void CalcCluster::Update() {
    if (clust_data_n != nullptr)delete clust_data_n;


    clust_data_n = (TH2F *) clust_data->Clone("NormarizedClusterSize");
    clust_data_n->SetTitle("ClusterSize;energy[eV];size");
    clust_data_n->SetDirectory(nullptr);   // EricaSim

    for (int ene = 0; ene < clust_data_n->GetXaxis()->GetNbins()+2; ene++) {
        for (int size = 0; size < clust_data_n->GetYaxis()->GetNbins()+2; size++) {
            if (size == 0) {
                continue;
            } else {
                clust_data_n->SetBinContent(ene, size,
                      clust_data_n->GetBinContent(ene, size - 1)
                      + clust_data_n->GetBinContent(ene, size));
            }
        }
    }

}

void CalcCluster::Load() {

    for (int ene = 0; ene < 14 + 1; ene++) {
        clust_data->Fill(ene, 1, 1e4);
    }

    ifstream ifs;
    ifs.open(fname);
    if (!ifs) {
        cerr << "can't find " << fname << endl;

    }


    int raw = 0;

    while (!ifs.eof()) {
        string line;
        getline(ifs, line);

        //skip header
        if (raw == 0) {
            raw++;
            continue;
        }

        if (raw >= CLUST_SIZE_MAX) {
            break;
        }
        istringstream stream(line);
        string field;
        vector<string> databuf;
        while (getline(stream, field, ',')) {
            databuf.push_back(field);
        }
        databuf.push_back(field);

        for (int col = 1; col < databuf.size(); col++) {
            clust_data->Fill(col + ENERGY_MIN,
                             atof(databuf[0].c_str()),
                             atof(databuf[col].c_str()));
        }
        raw++;
    }
    ifs.close();

    cout << "CalcCluster finish loading a data file" << endl;

    Update();
}