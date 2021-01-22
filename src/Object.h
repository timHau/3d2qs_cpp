//
// Created by tim on 22.01.21.
//

#ifndef INC_3D2QS_SUNC_CPP_OBJECT_H
#define INC_3D2QS_SUNC_CPP_OBJECT_H

#include "json.hpp"

class Object
{
private:
    std::string _id;
    std::string _model_id;
    std::string _type;
    std::vector<double> _transform;
    std::vector<std::vector<double>> _bbox;
    void load_object();

public:
    Object(nlohmann::json &node);
};

#endif //INC_3D2QS_SUNC_CPP_OBJECT_H
