#include <iostream>
#include "Object.h"

Object::Object(const std::shared_ptr<cpptoml::table> &obj) {
    _label = *(obj->get_qualified_as<std::string>("label"));
    init_bbox(obj);
}

void Object::init_bbox(const std::shared_ptr<cpptoml::table> &obj) {
    auto box = obj->get_array_of<cpptoml::array>("bbox");

    for (int i = 0; i < 8; ++i) {
        cpptoml::option<std::vector<double>> point = (*box)[i]->get_array_of<double>();
        Eigen::Vector3d v((*point)[0], (*point)[1], (*point)[2]);
        _bbox.emplace_back(v);
    }
}

std::vector<Eigen::Vector3d> *Object::get_bbox() {
    return &_bbox;
}

std::string *Object::get_label() {
    return &_label;
}

bool Object::is_equal_to(Object obj_b) {
    return _bbox == *obj_b.get_bbox();
}

std::vector<int> Object::is_inside_bb(std::vector<Eigen::Vector3d> &obj_b_bbox) {
    // test which points from obj_b bounding box is inside this bounding box, returns indices

    // get the points from bottom and top face
    std::vector<Eigen::Vector3d> bbox_a = *get_bbox();
    Eigen::Vector3d b_1 = bbox_a[0];
    Eigen::Vector3d b_2 = bbox_a[1];
    Eigen::Vector3d t_2 = bbox_a[2];
    Eigen::Vector3d t_1 = bbox_a[3];
    Eigen::Vector3d b_4 = bbox_a[4];
    Eigen::Vector3d b_3 = bbox_a[5];
    Eigen::Vector3d t_3 = bbox_a[6];
    Eigen::Vector3d t_4 = bbox_a[7];

    // find normalized direction from bottom to top
    Eigen::Vector3d dir_1 = (t_1 - b_1);
    float size_1 = dir_1.norm();
    dir_1.normalize();

    // find normalized direction from left to right
    Eigen::Vector3d dir_2 = (b_2 - b_1);
    float size_2 = dir_2.norm();
    dir_2.normalize();

    // find normalized direction from front to back
    Eigen::Vector3d dir_3 = (b_4 - b_1);
    float size_3 = dir_3.norm();
    dir_3.normalize();

    // find center of bounding box
    Eigen::Vector3d cuboid_center = 1 / 2 * (b_2 + b_4 + t_1 - b_1);

    // direction vectors from center to points of bounding box
    std::vector<Eigen::Vector3d> dir_vecs;
    dir_vecs.reserve(obj_b_bbox.size());
    for (const Eigen::Vector3d &v : obj_b_bbox) {
        dir_vecs.emplace_back(v - cuboid_center);
    }

    std::vector<int> outside;
    for (int i = 0; i < dir_vecs.size(); ++i) {
        Eigen::Vector3d dir_vec = dir_vecs[i];
        auto res_1 = abs(dir_vec.dot(dir_1) * 2);
        auto res_2 = abs(dir_vec.dot(dir_2) * 2);
        auto res_3 = abs(dir_vec.dot(dir_3) * 2);
        if (res_1 > size_1 && res_2 > size_2 && res_3 > size_3) {
            // point is outside
            outside.emplace_back(i);
        }
    }

    // get indices of points inside
    const std::vector<int> indices = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> diff;
    std::set_difference(indices.begin(), indices.end(), outside.begin(), outside.end(),
                        std::inserter(diff, diff.begin()));

    return diff;
}

bool Object::is_tangent_to(Object &obj_b) {
    // test if obj_b bounding box is inside the face of this bounding box
    std::vector<Eigen::Vector3d> bbox_a = *get_bbox();
    const std::vector<std::vector<Eigen::Vector3d>> faces = {
            {bbox_a[0], bbox_a[1], bbox_a[2], bbox_a[3]},  // vorne
            {bbox_a[3], bbox_a[2], bbox_a[6], bbox_a[7]},  // oben
            {bbox_a[1], bbox_a[5], bbox_a[6], bbox_a[2]},  // rechts
            {bbox_a[4], bbox_a[5], bbox_a[1], bbox_a[0]},  // unten
            {bbox_a[4], bbox_a[0], bbox_a[3], bbox_a[7]},  // links
            {bbox_a[5], bbox_a[4], bbox_a[7], bbox_a[6]},  // hinten
    };

    std::vector<Eigen::Vector3d> normals = {
            (bbox_a[2] - bbox_a[1]).cross(bbox_a[0] - bbox_a[1]),
            (bbox_a[6] - bbox_a[5]).cross(bbox_a[1] - bbox_a[5]),
            (bbox_a[6] - bbox_a[2]).cross(bbox_a[3] - bbox_a[2]),
            (bbox_a[0] - bbox_a[1]).cross(bbox_a[5] - bbox_a[1]),
            (bbox_a[3] - bbox_a[0]).cross(bbox_a[4] - bbox_a[0]),
            (bbox_a[7] - bbox_a[4]).cross(bbox_a[5] - bbox_a[4]),
    };
    for (Eigen::Vector3d v : normals) {
        v.normalize();
    }

    std::vector<Eigen::Vector3d> inside_face;
    for (const Eigen::Vector3d &v : *obj_b.get_bbox()) {
        for (int i = 0; i < faces.size(); ++i) {
            std::vector<Eigen::Vector3d> face = faces[i];
            // get point that is on face
            Eigen::Vector3d a = face[0];
            // get vector from that point to the point that is tested
            Eigen::Vector3d b = (a - v);
            // test if this vector is one the plane <==>  <(a-v), normal> = 0
            bool is_inside = b.dot(normals[i]) == 0;
            if (is_inside) {
                inside_face.emplace_back(i);
            }
        }
    }

    // if inside_face is not empty -> there exists point tangent
    return !inside_face.empty();
}

std::string Object::relation_to(Object obj_b) {
    // calculates the relation to obj_b based on their bounding boxes
    if (is_equal_to(obj_b)) {
        return "EQ";
    }

    // indices from obj_b that are inside this bounding box
    std::vector<int> inside_indices = is_inside_bb(*obj_b.get_bbox());

    // check disjoint
    if (inside_indices.empty()) {
        return "DC";
    }

    // check for partial intersection
    if (!inside_indices.empty() && inside_indices.size() < 8) {
        if (is_tangent_to(obj_b)) {
            return "EC";
        } else {
            return "PO";
        }
    }

    // check obj_b contained in this bounding box
    if (inside_indices.size() == 8) {
        if (is_tangent_to(obj_b)) {
            return "TPPc";
        }
        return "NTTPc";
    }

    // check if this is contained in obj_b
    std::vector<int> inside_indices_c = obj_b.is_inside_bb(*get_bbox());
    if (inside_indices_c.size() == 8) {
        if (is_tangent_to(obj_b)) {
            return "TPP";
        }
        return "NTPP";
    }

    return std::string();
}
