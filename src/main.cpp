#include <iostream>
#include "OBJ_Loader.h"
#include "json.hpp"
#include "boost/filesystem.hpp"
#include "Object.h"

using nlohmann::json;
using nlohmann::basic_json;

int main() {

    boost::filesystem::path obj_path("../data/object/41/41.vhacd.obj");

    objl::Loader loader;
    loader.LoadFile(obj_path.string());

    for (auto & mesh : loader.LoadedMeshes) {
        // std::cout << "Name: " << mesh.MeshName << std::endl;
        for (auto & vert : mesh.Vertices) {
            /*
            std::cout << "X: " << vert.Position.X << ' ';
            std::cout << "Y: " << vert.Position.Y << ' ';
            std::cout << "Z: " << vert.Position.Z << std::endl;
             */
        }
    }

    std::string scene_id = "0004d52d1aeeb8ae6de39d6bd993e992";
    std::string house_path = "../data/house/" + scene_id + "/house.json";
    std::ifstream house_stream(house_path);
    json j;
    house_stream >> j;

    basic_json levels = j["levels"];
    for (auto & level : levels) {
        basic_json nodes = level["nodes"];
        for (auto & node: nodes) {
            Object o(node);
        }
    }

    return 0;
}
