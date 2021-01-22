//
// Created by tim on 22.01.21.
//

#include "Object.h"
#include "boost/filesystem.hpp"
#include "OBJ_Loader.h"

Object::Object(nlohmann::json &node) {
    std::vector<double> max = node["bbox"]["max"];
    std::vector<double> min = node["bbox"]["min"];
    _bbox = {
            {min[0], min[1], max[2]},   // vorne, unten, links
            {max[0], min[1], max[2]},   // vorne, unten, rechts
            {max[0], max[1], max[2]},   // vorne, oben, rechts
            {min[0], max[1], max[2]},   // vorne, oben, links
            {min[0], min[1], min[2]},   // hinten, unten, links
            {max[0], min[1], min[2]},   // hinten, unten, rechts
            {max[0], max[1], min[2]},   // hinten, oben, rechts
            {min[0], max[1], min[2]}    // hinten, oben, links
    };
    _model_id = node["modelId"];
    _type = node["type"];
    if (node.contains("transform")) {
        std::vector<double> t = node["transform"];
        _transform = t;
    }
    load_object();
}

void Object::load_object() {
    boost::filesystem::path obj_path("../data/object/" + _model_id + "/" + _model_id + ".vhacd.obj");

    objl::Loader loader;
    loader.LoadFile(obj_path.string());

    for (auto & mesh : loader.LoadedMeshes) {
        std::cout << "Name: " << mesh.MeshName << std::endl;
        for (auto & vert : mesh.Vertices) {
            /*
            std::cout << "X: " << vert.Position.X << ' ';
            std::cout << "Y: " << vert.Position.Y << ' ';
            std::cout << "Z: " << vert.Position.Z << std::endl;
             */
        }
    }

}
