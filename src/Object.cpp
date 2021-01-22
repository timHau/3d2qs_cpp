//
// Created by tim on 22.01.21.
//

#include "Object.h"
#include "boost/filesystem.hpp"
#include <Eigen/Dense>
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
        _transform <<  t[0],  t[1],  t[2],  t[3],
                       t[4],  t[5],  t[6],  t[7],
                       t[8],  t[9],  t[10], t[11],
                       t[12], t[13], t[14], t[15];
    }
    load_object();
}

void Object::load_object() {
    boost::filesystem::path obj_path("../data/object/" + _model_id + "/" + _model_id + ".vhacd.obj");

    objl::Loader loader;
    loader.LoadFile(obj_path.string());

    for (auto & m : loader.LoadedMeshes) {
        for (auto & vert : m.Vertices) {
            Eigen::Vector3d point;
            point << vert.Position.X, vert.Position.Y, vert.Position.Z;
            _mesh.push_back(point);
        }

        for (auto & ind : m.Indices) {
            _indices.push_back(ind);
        }
    }
}
