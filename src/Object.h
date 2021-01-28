#ifndef INC_3D2QS_SUNC_CPP_OBJECT_H
#define INC_3D2QS_SUNC_CPP_OBJECT_H

#include "cpptoml.h"
#include <Eigen/Dense>

class Object {
private:
    std::string _label;
    std::string _id;
    std::vector<Eigen::Vector3d> _bbox;

    void init_bbox(const std::shared_ptr<cpptoml::table> &obj);

    bool is_equal_to(Object obj_b);

    [[nodiscard]] bool is_tangent_to(Object &obj_b);

    int count_inside_bb(std::vector<Eigen::Vector3d> &obj_b_bbox);

public:
    explicit Object(const std::shared_ptr<cpptoml::table> &obj);

    std::vector<Eigen::Vector3d> *get_bbox();

    std::string *get_label();

    std::string *get_id();

    std::string relation_to(Object obj_b);
};

#endif //INC_3D2QS_SUNC_CPP_OBJECT_H
