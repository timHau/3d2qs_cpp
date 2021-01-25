#include <iostream>
#include <fstream>
#include "json.hpp"
#include "cpptoml.h"
#include "Object.h"


std::vector<std::pair<Object, Object>>
cartesian_product(const std::vector<Object> &objs_a, const std::vector<Object> &objs_b) {
    std::vector<std::pair<Object, Object>> product;
    for (auto &i : objs_a) {
        for (auto &j : objs_b) {
            product.emplace_back(i, j);
        }
    }
    return product;
}

int main() {
    /*
    // load house.json
    std::string scene_id = "0004d52d1aeeb8ae6de39d6bd993e992";
    std::string house_path = "../data/house/" + scene_id + "/house.json";
    std::ifstream house_stream(house_path);
    json j;
    house_stream >> j;
    */

    auto config = cpptoml::parse_file("../data/test.toml");
    auto val = config->get_qualified_as<std::string>("dataset.name");
    std::vector<Object> objects;
    for (const auto& obj : *config->get_table_array("object")) {
        Object object(obj);
        objects.emplace_back(object);
    }

    for (auto &obj_pair : cartesian_product(objects, objects)) {
        Object obj_a = obj_pair.first;
        Object obj_b = obj_pair.second;
        std::string rel = obj_a.relation_to(obj_b);
        std::cout << rel << std::endl;
    }

    /*
    basic_json levels = j["levels"];
    std::vector<Object_SUN> objects;
    for (auto &level : levels) {
        basic_json nodes = level["nodes"];
        for (auto &node: nodes) {
            if (node["valid"] == 1) {
                Object_SUN o(node);
                objects.push_back(o);
            }
        }
    }

    for (auto &obj_pair : cartesian_product(objects, objects)) {
        Object_SUN obj_a = obj_pair.first;
        Object_SUN obj_b = obj_pair.second;
        std::string rel = obj_a.relation_to(obj_b);
        std::cout << rel << std::endl;
    }
    */

    return 0;
}


