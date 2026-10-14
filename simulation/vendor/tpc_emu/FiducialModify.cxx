//
// Created by ynakamura on 19/07/11.
//

#include "FiducialModify.h"
#include "SM_Log.h"
#include <TFile.h>
#include <TH2F.h>
#include <TVector3.h>

using namespace smlib;
using namespace std;

double FiducialModify::func_stripmap(double x, double y) {
    // Ver. 1
    /*
    double par[] = {1.90173e+02,
                    1.18425e+00,
                    1.63625e-03,
                    -1.27936e-05,
                    -5.80364e-07,
                    4.38410e-08,
                    -2.41948e-08,
                    3.89154e-11,
                    -4.01303e-10,
                    3.32134e-10,
                    -2.66988e-13,
                    2.13775e-12,
                    3.12734e-12,
                    3.63496e-17,
                    -9.77308e-16,
                    -1.13239e-14,
                    -1.86882e-14};
*/
    //Ver. 2
    double par[] = {
            191.975,
            1.22042,
            0.00099864,
            -8.66135e-06,
            6.94858e-07,
            3.00097e-08,
            -3.54556e-10,
            4.4894e-11,
            -1.24713e-10,
            -1.04556e-11,
            -1.42219e-13,
            1.68125e-13,
            5.94958e-14,
            -2.10411e-16,
            1.77206e-15,
            -2.30926e-15,
            -5.02117e-17
    };

    constexpr int FUNC_DIM = 7;
    int parid = 0;
    double sol = 0;
    for (int i = 0; i <= FUNC_DIM; i++) {
        for (int j = 0; j <= i; j++) {
            if (i != 0 && j % 2 == 0) {
                continue;
            }
            sol += par[parid] * pow(x, j) * pow(y, i - j);
            parid++;
        }
    }
    return sol;
}

double FiducialModify::func_timemap(double x, double y) {
    //Ver. 1
    /* double par[] = {-2.37492e-01,
                     -6.84273e-03,
                     8.47431e-04,
                     -2.19483e-06,
                     -1.10124e-05,
                     1.12413e-07,
                     7.55500e-08,
                     -1.70147e-09,
                     1.56029e-10,
                     -2.80789e-10,
                     1.37625e-11,
                     -4.87871e-12,
                     5.32649e-13,
                     -3.81742e-14,
                     4.53829e-15,
                     -3.74470e-15,
                     -4.03548e-16,
                     2.53693e-17,
                     5.37379e-17,
                     7.69162e-17};*/

    //Ver.2
    double par[] = {
            -0.250541,
            -0.00603632,
            0.000831295,
            -1.28778e-06,
            -1.08784e-05,
            6.67333e-08,
            7.51034e-08,
            -9.61073e-10,
            1.25072e-10,
            -2.80702e-10,
            9.00059e-12,
            -4.49893e-12,
            5.35154e-13,
            -2.54867e-14,
            4.50227e-15,
            -1.35703e-15,
            -4.0728e-16,
            1.32075e-17,
            5.96326e-17,
            3.72502e-17
    };

    constexpr int FUNC_DIM = 7;
    int parid = 0;
    double sol = 0;
    for (int i = 0; i <= FUNC_DIM; i++) {
        for (int j = 0; j <= i; j++) {
            if (j % 2 == 1) {
                continue;
            }
            sol += par[parid] * pow(x, j) * pow(y, i - j);
            parid++;
        }
    }
    return sol;
}

double FiducialModify::func_xmap(double strip, double time) {
    //Ver. 1
    /*
    double par[] = {-1.49890e-02,
                    8.02648e-01,
                    -1.16537e-02,
                    3.95700e-03,
                    5.96625e-07,
                    -8.55685e-04,
                    1.97018e-07,
                    -9.09653e-05,
                    1.26148e-08,
                    -7.76228e-12,
                    7.53094e-06,
                    1.26148e-08,
                    -7.76228e-12,
                    1.63364e-06,
                    -2.50168e-09,
                    8.73091e-13,
                    3.73308e-16};
*/
    //Ver. 2
/*    double par[] = {
            999.985 - 1000.,
            0.805615,
            -0.023493,
            0.0147222,
            5.84056e-07,
            -0.00447488,
            2.20528e-07,
            0.000366137,
            8.39042e-09,
            1.56484e-10,
            7.17793e-07,
            8.73607e-09,
            -1.72523e-10,
            8.82597e-12,
            -2.38563e-09,
            8.86323e-13,
            3.80075e-16,
    };*/

    //Ver. 3
    double par[] = {
            999.991 - 1000.,
            0.799519,
            -0.00150007,
            -0.00301921,
            1.1988e-07,
            0.00129422,
            -4.26368e-07,
            -0.000448604,
            3.21259e-07,
            -6.97629e-12,
            4.22497e-05,
            -5.86109e-08,
            6.01642e-12,
            2.99431e-09,
            1.66761e-09,
            -8.70765e-15,
            1.18402e-16
    };


    constexpr int FUNC_DIM = 7;
    int parid = 0;
    double sol = 0;
    for (int i = 0; i <= FUNC_DIM; i++) {
        for (int j = 0; j <= i; j++) {
            if (i != 0 && j % 2 == 0) {
                continue;
            }
            sol += par[parid] * pow(strip - 192., j) * pow(time, i - j);
            parid++;
        }
    }
    return sol;
}

double FiducialModify::func_ymap(double strip, double time) {
    //Ver. 1
    /*
    double par[] = {-2.37492e-01,
                    -6.84273e-03,
                    8.47431e-04,
                    -2.19483e-06,
                    -1.10124e-05,
                    1.12413e-07,
                    7.55500e-08,
                    -1.70147e-09,
                    1.56029e-10,
                    -2.80789e-10,
                    1.37625e-11,
                    -4.87871e-12,
                    5.32649e-13,
                    -3.81742e-14,
                    4.53829e-15,
                    -3.74470e-15,
                    -4.03548e-16,
                    2.53693e-17,
                    5.37379e-17,
                    7.69162e-17};
*/
    //Ver. 2
    double par[] = {
            26.7468,
            40.671,
            0.559319,
            -2.69711e-05,
            -0.181545,
            2.87845e-05,
            0.0423401,
            -2.0849e-05,
            7.14507e-10,
            -0.005165,
            -9.33996e-07,
            1.48075e-09,
            0.000662854,
            9.10044e-10,
            -3.13218e-13,
            -1.96431e-17,
            -4.90925e-05,
            8.39587e-08,
            -6.45875e-11,
            -1.15122e-14
    };
    constexpr int FUNC_DIM = 7;
    int parid = 0;
    double sol = 0;
    for (int i = 0; i <= FUNC_DIM; i++) {
        for (int j = 0; j <= i; j++) {
            if (j % 2 == 1) {
                continue;
            }
            sol += par[parid] * pow(strip - 192., j) * pow(time, i - j);
            parid++;
        }
    }
    return sol;
}


void FiducialModify::Init(string fname) {
    TFile *fdata = new TFile(fname.c_str());

    if (fdata->IsZombie()) {
        SM_LOGGER(fatal) << "the file , " << fname << " was not found";
    }

    TH2F *h_tmp;
    string xydataname[] = {"xmap", "ymap"};
    string stdataname[] = {"stripmap", "timemap"};


    for (int st = 0; st < 2; st++) {
        h_tmp = NULL;
        fdata->GetObject(stdataname[st].c_str(), h_tmp);
        if (h_tmp == NULL) {
            SM_LOGGER(fatal) << "the hist , " << stdataname[st] << " was not found";
            exit(-1);
        }
        h_2dmap_st[st] = h_tmp;
    }

    for (int xy = 0; xy < 2; xy++) {
        h_tmp = NULL;
        fdata->GetObject(xydataname[xy].c_str(), h_tmp);
        if (h_tmp == NULL) {
            SM_LOGGER(fatal) << "the hist , " << xydataname[xy] << " was not found";
            exit(-1);
        }
        h_2dmap_xy[xy] = h_tmp;
    }

    f_mapmode = true;
}

//-----2D transformation-----

void FiducialModify::DetectorToReal_2D(double strip, double time, double *x, double *y) {
    if (f_mapmode) {
        int sbin = h_2dmap_xy[0]->GetXaxis()->FindBin(strip);
        int bbin = h_2dmap_xy[0]->GetYaxis()->FindBin(time);

        *x = h_2dmap_xy[0]->GetBinContent(sbin, bbin);
        *y = h_2dmap_xy[1]->GetBinContent(sbin, bbin);
    } else {
        *x = func_xmap(strip, time);
        *y = func_ymap(strip, time);
    }

}

void FiducialModify::RealToDetector_2D(double x, double y, double *strip, double *time) {
    if (f_mapmode) {
        int xbin = h_2dmap_st[0]->GetXaxis()->FindBin(x);
        int ybin = h_2dmap_st[0]->GetYaxis()->FindBin(y);

        *strip = h_2dmap_st[0]->GetBinContent(xbin, ybin);
        *time = h_2dmap_st[1]->GetBinContent(xbin, ybin);
    } else {
        *strip = func_stripmap(x, y);
        *time = func_timemap(x, y);
    }
}

void FiducialModify::NotModToReal_2D(double nm_x, double nm_y,
                                     double *x, double *y) {
    DetectorToReal_2D(
            (nm_x + 128. * 0.8 * 1.5) / 0.8,
            (nm_y - 26.) / DRIFT_VELO_TEMPLATE,
            x, y);
}


//-----3D transformation-----

void FiducialModify::DetectorToReal_3D_byBin(int sbin_a, int sbin_c, int tbin,
                                             double *x, double *y, double *z) {

    SM_LOGGER(info) << "this function will be deleted";

    if (!f_mapmode) {
        SM_LOGGER(error) << "this function must call Init([mapdata])";
        return;
    }

    double ztemp[2];
    *x = h_2dmap_xy[0]->GetBinContent(sbin_a, tbin);
    *y = h_2dmap_xy[0]->GetBinContent(sbin_c, tbin);
    ztemp[0] = h_2dmap_xy[1]->GetBinContent(sbin_a, tbin);
    ztemp[1] = h_2dmap_xy[1]->GetBinContent(sbin_c, tbin);

    if (ztemp[0] > ztemp[1]) {
        *z = ztemp[0];
    } else {
        *z = ztemp[1];
    }
}

void FiducialModify::DetectorToReal_3D(double strip_a, double strip_c, double time,
                                       double *x, double *y, double *z) {

    if (f_mapmode) {
        int sbin_a = h_2dmap_xy[0]->GetXaxis()->FindBin(strip_a);
        int sbin_c = h_2dmap_xy[0]->GetXaxis()->FindBin(strip_c);
        int tbin = h_2dmap_xy[0]->GetYaxis()->FindBin(time);
        DetectorToReal_3D_byBin(sbin_a, sbin_c, tbin, x, y, z);
    } else {
        double ztemp[2];
        *x = func_xmap(strip_a, time);
        *y = func_xmap(strip_c, time);
        ztemp[0] = func_ymap(strip_a, time);
        ztemp[1] = func_ymap(strip_c, time);

        if (ztemp[0] > ztemp[1]) {
            *z = ztemp[0];
        } else {
            *z = ztemp[1];
        }
    }
}

/*!
 * caution: DetectorToReal_3D*RealToDetector_3D is not identity mapping
 * @param x
 * @param y
 * @param z
 * @param strip_a
 * @param strip_c
 * @param time
 * @param drift_velo drift velocity[mm/us]
 */
void FiducialModify::RealToDetector_3D(double x, double y, double z,
                                       double *strip_a, double *strip_c, double *time, double drift_velo) {
    double amp = DRIFT_VELO_TEMPLATE / drift_velo;
    double ttemp[2];

    if (f_mapmode) {
        int xbin_a = h_2dmap_st[0]->GetXaxis()->FindBin(x);
        int xbin_c = h_2dmap_st[0]->GetXaxis()->FindBin(y);
        int ybin = h_2dmap_st[0]->GetYaxis()->FindBin(z);

        *strip_a = h_2dmap_st[0]->GetBinContent(xbin_a, ybin);
        *strip_c = h_2dmap_st[0]->GetBinContent(xbin_c, ybin);
        ttemp[0] = h_2dmap_st[1]->GetBinContent(xbin_a, ybin);
        ttemp[1] = h_2dmap_st[1]->GetBinContent(xbin_c, ybin);
    } else {
        *strip_a = func_stripmap(x, z);
        *strip_c = func_stripmap(y, z);
        ttemp[0] = func_timemap(x, z);
        ttemp[1] = func_timemap(x, z);
    }

    if (ttemp[0] < ttemp[1]) {
        *time = ttemp[0] * amp;
    } else {
        *time = ttemp[1] * amp;
    }
}


void FiducialModify::NotModToReal_3D(double nm_x, double nm_y, double nm_z,
                                     double *x, double *y, double *z) {
    DetectorToReal_3D(
            (nm_x + 128. * 0.8 * 1.5) / 0.8,
            (nm_y + 128. * 0.8 * 1.5) / 0.8,
            (nm_z - 26.) / DRIFT_VELO_TEMPLATE,
            x, y, z);
}

void FiducialModify::NotModToReal_3D_PosAndVector(
        const double *nm_pos, const double *nm_vec,
        double *pos, double *vec) {

    SM_LOGGER(info) << "this function is under construction";

    if (!f_mapmode) {
        SM_LOGGER(error) << "this function must call Init([mapdata])";
        return;
    }


    double strip_a, strip_c, time;
    strip_a = (nm_pos[0] + 128. * 0.8 * 1.5) / 0.8;
    strip_c = (nm_pos[1] + 128. * 0.8 * 1.5) / 0.8;
    time = (nm_pos[2] - 26.) / DRIFT_VELO_TEMPLATE;

    double ztemp[2];

    int sbin_a = h_2dmap_xy[0]->GetXaxis()->FindBin(strip_a);
    int sbin_c = h_2dmap_xy[0]->GetXaxis()->FindBin(strip_c);
    int tbin = h_2dmap_xy[0]->GetYaxis()->FindBin(time);
    DetectorToReal_3D_byBin(sbin_a, sbin_c, tbin, pos, pos + 1, pos + 2);

    double tmp[3][3];
    TVector3 base[3];
    TVector3 nm(nm_vec[0], nm_vec[1], nm_vec[2]);
    TVector3 sol;
    for (int i = 0; i < 3; i++) {
        int offset[3] = {0, 0, 0};
        offset[i] = 1;
        DetectorToReal_3D_byBin(sbin_a + offset[0],
                                sbin_c + offset[1], tbin + offset[2], tmp[i], tmp[i] + 1, tmp[i] + 2);

        double binwidth[2] = {h_2dmap_xy[0]->GetXaxis()->GetBinWidth(1),
                              h_2dmap_xy[0]->GetYaxis()->GetBinWidth(1)};

        base[i].SetXYZ((tmp[i][0] - pos[0]) / binwidth[0],
                       (tmp[i][1] - pos[1]) / binwidth[0],
                       (tmp[i][2] - pos[2]) / binwidth[1] / DRIFT_VELO_TEMPLATE);

    }

    sol = nm.X() * base[0] + nm.Y() * base[1] + nm.Z() * base[2];

    sol = sol.Unit();
    vec[0] = sol.X();
    vec[1] = sol.Y();
    vec[2] = sol.Z();
}