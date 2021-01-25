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
    std::vector<Eigen::Vector3d> _vertices;
    std::vector<std::vector<int>> _faces; // list of list size 3 containing vertex indices
    void load_object();
    bool is_equal_to(const Object& obj_b);
    bool is_tangent_to(const Object& obj_b) const;
    std::vector<int> inside_bb(const std::vector<Eigen::Vector3d>& obj_b_bbox) const;

public:
    explicit Object(nlohmann::json &node);
    std::vector<Eigen::Vector3d> get_bbox() const;
    std::string relation_to(const Object& obj_b);
};

#endif //INC_3D2QS_SUNC_CPP_OBJECT_H
