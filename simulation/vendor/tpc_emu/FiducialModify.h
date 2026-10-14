//
// Created by ynakamura on 19/07/11.
//

#ifndef LIBS_FIDUCIALMODIFY_H
#define LIBS_FIDUCIALMODIFY_H

#include <string>

class TH2F;

/*!
 * real: real space with fiducial modification [mm]
 * detector: detector space ([strip], [us]), analysis with typical drift velocity.
 * notmod: detector space calibrated by drift velocity [mm]
 *
 * @date 2019/08/10 (update) function mode was added
 * @date 2019/08/19 (debug) function parameters were modified (Ver.2)
 * @date 2019/08/20 (debug) xmap function parameters were modified (Ver.3)
 */
class FiducialModify {
    TH2F *h_2dmap_st[2];   //real -> detector(strip, time)
    TH2F *h_2dmap_xy[2];   //detector -> real(x,y)
    static constexpr double DRIFT_VELO_TEMPLATE = 44.0; //[mm/us]
    bool f_mapmode;

    double func_stripmap(double x, double y);
    double func_timemap(double x, double y);
    double func_xmap(double strip, double time);
    double func_ymap(double strip, double time);


public:
    FiducialModify():f_mapmode(false){};
    void Init(std::string fname);

    //-----2D transformation-----

    void DetectorToReal_2D(double strip, double time, double *x, double *y);

    void RealToDetector_2D(double x, double y, double *strip, double *time);

    void NotModToReal_2D(double nm_x, double nm_y, double *x, double *y);


    //-----3D transformation-----

    void DetectorToReal_3D(double strip_a, double strip_c, double time,
                           double *x, double *y, double *z);

    void DetectorToReal_3D_byBin(int sbin_a, int sbin_c, int tbin,
                                           double *x, double *y, double *z);

    void RealToDetector_3D(double x, double y, double z,
                           double *strip_a, double *strip_c, double *time, double drift_velo = DRIFT_VELO_TEMPLATE);

    void NotModToReal_3D(double nm_x, double nm_y, double nm_z,
                           double *x, double *y, double *z);

    void NotModToReal_3D_PosAndVector(
            const double *nm_pos, const double *nm_vec,
                         double *pos, double *vec);
};


#endif // LIBS_FIDUCIALMODIFY_H
