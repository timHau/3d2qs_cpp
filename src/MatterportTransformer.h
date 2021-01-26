//
// Created by tim on 26.01.21.
//

#ifndef INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
#define INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H

#include "happly.h"
#include <iostream>
#include <fstream>
#include "json.hpp"
#include "cpptoml.h"

class MatterportTransformer {
public:
    static void transform(const std::string& path);
};


#endif //INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
