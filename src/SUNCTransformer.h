//
// Created by tim on 25.01.21.
//

#ifndef INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H
#define INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H

#include <iostream>
#include <fstream>
#include "json.hpp"
#include "cpptoml.h"
#include <Eigen/Dense>


class SUNCTransformer {
public:
    static void transform(const std::string& path);
};


#endif //INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H
