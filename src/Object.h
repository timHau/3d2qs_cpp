//
// Created by tim on 22.01.21.
//

#ifndef INC_3D2QS_SUNC_CPP_OBJECT_H
#define INC_3D2QS_SUNC_CPP_OBJECT_H

#include "json.hpp"

class Object
{
private:
    std::vector<double> _bbox;
public:
    Object(nlohmann::json &node);
};

#endif //INC_3D2QS_SUNC_CPP_OBJECT_H
