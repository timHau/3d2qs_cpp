#include <iostream>
#include "Object.h"

Object::Object(const std::shared_ptr<cpptoml::table> &obj) {
    _label = *(obj->get_qualified_as<std::string>("label"));
    _id = *(obj->get_qualified_as<std::string>("id"));
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

std::string *Object::get_id() {
    return &_id;
}

bool Object::is_equal_to(Object obj_b) {
    return _bbox == *obj_b.get_bbox();
}

int Object::count_inside_bb(std::vector<Eigen::Vector3d> &obj_b_bbox) {
    // test which points from obj_b bounding box is inside this bounding box, returns indices

    // get the points from bottom and top face
    std::vector<Eigen::Vector3d> bbox_a = *get_bbox();
    Eigen::Vector3d V = bbox_a[0];
    Eigen::Vector3d A = bbox_a[1];
    Eigen::Vector3d B = bbox_a[3];
    Eigen::Vector3d C = bbox_a[4];

    Eigen::Vector3d AV = (A-V);
    Eigen::Vector3d BV = (B-V);
    Eigen::Vector3d CV = (C-V);

    // number of points in obj_bs bounding box that are inside obj_as bounding box
    int count = 0;
    for (const Eigen::Vector3d & P : obj_b_bbox) {
        bool t_1 = V.dot(AV) < P.dot(AV) && P.dot(AV) < A.dot(AV);
        bool t_2 = V.dot(BV) < P.dot(BV) && P.dot(BV) < B.dot(BV);
        bool t_3 = V.dot(CV) < P.dot(CV) && P.dot(CV) < C.dot(CV);
        if (t_1 && t_2 && t_3) {
            count += 1;
        }
    }
    return count;
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
            // TODO test this
            try {
                Eigen::Vector3d a = face[0];
                // get vector from that point to the point that is tested
                Eigen::Vector3d b = (a - v);
                // test if this vector is one the plane <==>  <(a-v), normal> = 0
                bool is_inside = b.dot(normals[i]) == 0;
                if (is_inside) {
                    inside_face.emplace_back(i);
                }
            } catch(...) {
                std::cout << face[0] << std::endl;
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

    // number of points from obj_b that are inside this bounding box
    int inside_count = count_inside_bb(*obj_b.get_bbox());

    // check for partial intersection
    if (0 < inside_count && inside_count < 8) {
        if (is_tangent_to(obj_b)) {
            return "EC";
        } else {
            return "PO";
        }
    }

    // check disjoint
    if (inside_count == 0) {
        return "DC";
    }

    // check obj_b contained in this bounding box
    if (inside_count == 8) {
        if (is_tangent_to(obj_b)) {
            return "TPPc";
        }
        return "NTTPc";
    }

    // check if this is contained in obj_b
    int inside_count_c = obj_b.count_inside_bb(*get_bbox());
    if (inside_count_c == 8) {
        if (is_tangent_to(obj_b)) {
            return "TPP";
        }
        return "NTPP";
    }

    return std::string();
}
