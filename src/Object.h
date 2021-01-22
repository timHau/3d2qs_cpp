//
// Created by tim on 22.01.21.
//

#ifndef INC_3D2QS_SUNC_CPP_OBJECT_H
#define INC_3D2QS_SUNC_CPP_OBJECT_H

#include "json.hpp"
#include <Eigen/Dense>

class Object
{
private:
    std::string _id;
    std::string _model_id;
    std::string _type;
    Eigen::Matrix4d _transform;
    std::vector<Eigen::Vector3d> _bbox;
    std::vector<Eigen::Vector3d> _mesh;
    std::vector<double> _indices;
    void load_object();

public:
    Object(nlohmann::json &node);
};

#endif //INC_3D2QS_SUNC_CPP_OBJECT_H
