//
// Created by tim on 22.01.21.
//

#include "Object.h"
#include "boost/filesystem.hpp"
#include <Eigen/Dense>
#include <algorithm>
#include "OBJ_Loader.h"

Object::Object(nlohmann::json &node) {
    std::vector<double> max = node["bbox"]["max"];
    std::vector<double> min = node["bbox"]["min"];
    _bbox = {
            Eigen::Vector3d(min[0], min[1], max[2]),   // vorne, unten, links
            Eigen::Vector3d(max[0], min[1], max[2]),   // vorne, unten, rechts
            Eigen::Vector3d(max[0], max[1], max[2]),   // vorne, oben, rechts
            Eigen::Vector3d(min[0], max[1], max[2]),   // vorne, oben, links
            Eigen::Vector3d(min[0], min[1], min[2]),   // hinten, unten, links
            Eigen::Vector3d(max[0], min[1], min[2]),   // hinten, unten, rechts
            Eigen::Vector3d(max[0], max[1], min[2]),   // hinten, oben, rechts
            Eigen::Vector3d(min[0], max[1], min[2])    // hinten, oben, links
    };
    _model_id = node["modelId"];
    _type = node["type"];
    if (node.contains("transform")) {
        std::vector<double> t = node["transform"];
        _transform << t[0], t[1], t[2], t[3],
                t[4], t[5], t[6], t[7],
                t[8], t[9], t[10], t[11],
                t[12], t[13], t[14], t[15];
    }
    load_object();
}

void Object::load_object() {
    boost::filesystem::path obj_path("../data/object/" + _model_id + "/" + _model_id + ".obj");

    objl::Loader loader;
    loader.LoadFile(obj_path.string());

    for (auto &m : loader.LoadedMeshes) {
        for (auto &vert : m.Vertices) {
            Eigen::Vector3d point;
            point << vert.Position.X, vert.Position.Y, vert.Position.Z;
            _vertices.push_back(point);
        }

        for (int i = 0; i < m.Indices.size(); i += 3) {
            std::vector<int> face;
            face.push_back(m.Indices[i + 0]);
            face.push_back(m.Indices[i + 1]);
            face.push_back(m.Indices[i + 2]);
            _faces.push_back(face);
        }
    }
}

std::vector<Eigen::Vector3d> Object::get_bbox() const {
    return _bbox;
}

bool Object::is_equal_to(const Object &obj_b) {
    return _bbox == obj_b.get_bbox();
}

bool Object::is_tangent_to(const Object &obj_b) const {
    // test if obj_b bounding box is inside the face of this bounding box
    const std::vector<Eigen::Vector3d> bbox_a = get_bbox();
    const std::vector<std::vector<Eigen::Vector3d>> faces = {
            { bbox_a[0], bbox_a[1], bbox_a[2], bbox_a[3] },  // vorne
            { bbox_a[3], bbox_a[2], bbox_a[6], bbox_a[7] },  // oben
            { bbox_a[1], bbox_a[5], bbox_a[6], bbox_a[2] },  // rechts
            { bbox_a[4], bbox_a[5], bbox_a[1], bbox_a[0] },  // unten
            { bbox_a[4], bbox_a[0], bbox_a[3], bbox_a[7] },  // links
            { bbox_a[5], bbox_a[4], bbox_a[7], bbox_a[6] },  // hinten
    };

    std::vector<Eigen::Vector3d> inside_face;
    for (const Eigen::Vector3d & v : obj_b.get_bbox()) {
        for (const std::vector<Eigen::Vector3d> & face : faces) {
            // test if v is inside face
            Eigen::Vector3d v_1 = face[0];
            Eigen::Vector3d v_2 = face[1];
            Eigen::Vector3d v_3 = face[2];
            Eigen::Vector3d v_4 = face[3];
            // project v inside v_2 - v_1 (vector pointing right)
            Eigen::Vector3d vec_right = v_2 - v_1;
            double proj_right = v.dot(vec_right);
            bool t_1 = v_1.dot(vec_right) <= proj_right && proj_right <= v_2.dot(vec_right);
            // project v inside v_4 - v_1 (vector pointing up)
            Eigen::Vector3d vec_up = v_4 - v_1;
            double proj_up = v.dot(vec_up);
            bool t_2 = v_1.dot(vec_up) <= proj_up && proj_up <= v_4.dot(vec_up);
            // test if v is inside (proj_right) x (proj_up)
            if (t_1 && t_2) {
                inside_face.emplace_back(v);
            }
        }
    }

    // if inside_face is not empty -> there exists point tangent
    return !inside_face.empty();
}

std::string Object::relation_to(const Object &obj_b) {
    // calculates the relation to obj_b based on their bounding boxes
    if (is_equal_to(obj_b)) {
        return "EQ";
    }

    // indices from obj_b that are inside this bounding box
    std::vector<int> inside_indices = inside_bb(obj_b.get_bbox());

    // check disjoint
    if (inside_indices.empty()) {
        return "DC";
    }

    // check for partial intersection
    if (!inside_indices.empty() && inside_indices.size() < 8) {
       if (is_equal_to(obj_b)) {
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
    std::vector<int> inside_indices_c = obj_b.inside_bb(this->get_bbox());
    if (inside_indices_c.size() == 8){
        if (is_equal_to(obj_b))  {
            return "TPP";
        }
        return "NTPP";
    }

    return std::string();
}

std::vector<int> Object::inside_bb(const std::vector<Eigen::Vector3d>& obj_b_bbox) const {
    // test which points from obj_b bounding box is inside this bounding box, returns indices

    // get the points from bottom and top face
    std::vector<Eigen::Vector3d> bbox_a = get_bbox();
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
    for (const Eigen::Vector3d & v : obj_b_bbox) {
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

