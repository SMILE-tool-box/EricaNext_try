#ifndef GGGLOADER_H_
#define GGGLOADER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace GGG_LIB {
    double                      atof(const std::string str);
    int                         atoi(const std::string str);


    char                        get_one(std::string *str);
    std::string                 split(std::string *str, const char del);

    //remove double quotations from string
    void                        rmb_dblquot(std::string *str);

    int sign(int a);
}

#endif  // GGGLOADER_H_
