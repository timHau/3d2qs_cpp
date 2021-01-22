#include <iostream>
#include "OBJ_Loader.h"
#include "json.hpp"
#include "boost/filesystem.hpp"

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

    std::string sceneId = "0004d52d1aeeb8ae6de39d6bd993e992";
    std::string housePath = "../data/house/" + sceneId + "/house.json";
    std::ifstream houseStream(housePath);
    std::string houseJson( (std::istreambuf_iterator<char>(houseStream) ),
                      (std::istreambuf_iterator<char>() ));

    auto house = nlohmann::json::parse(houseJson);
    std::cout << house.dump(4) << std::endl;

    return 0;
}
