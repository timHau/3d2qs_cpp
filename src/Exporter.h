//
// Created by tau on 29.01.21.
//

#ifndef INC_3D2QS_SUNC_CPP_EXPORTER_H
#define INC_3D2QS_SUNC_CPP_EXPORTER_H

#include <iostream>
#include <string>
#include "cpptoml.h"
#include "happly.h"

class Exporter {
public:
    static void to_ply(const std::string& input);
};


#endif //INC_3D2QS_SUNC_CPP_EXPORTER_H
