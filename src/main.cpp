#include <iostream>
#include "cpptoml.h"
#include "Object.h"
#include "Utils.h"
#include "SUNCTransformer.h"


int main() {
    // load house.json
    std::string scene_id = "0004d52d1aeeb8ae6de39d6bd993e992";
    std::string house_path = "../data/house/" + scene_id + "/house.json";
    SUNCTransformer::transform(house_path);

    auto config = cpptoml::parse_file("../data/test.toml");
    auto val = config->get_qualified_as<std::string>("dataset.name");
    std::vector<Object> objects;
    for (const auto& obj : *config->get_table_array("object")) {
        Object object(obj);
        objects.emplace_back(object);
    }

    for (auto &obj_pair : utils::cartesian_product(objects, objects)) {
        Object obj_a = obj_pair.first;
        Object obj_b = obj_pair.second;
        std::string rel = obj_a.relation_to(obj_b);
        std::cout << rel << std::endl;
    }

    return 0;
}


