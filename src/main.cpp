#include <iostream>
#include <fstream>
#include "json.hpp"
#include "Object.h"

using nlohmann::json;
using nlohmann::basic_json;

std::vector<std::pair<Object, Object>> cartesian_product(std::vector<Object> objs_a, std::vector<Object> objs_b) {
    std::vector<std::pair<Object, Object>> product;
    for (auto &i : objs_a) {
        for (auto &j : objs_b) {
            product.emplace_back(i, j);
        }
    }

    return product;
}

int main() {
    // load house.json
    std::string scene_id = "0004d52d1aeeb8ae6de39d6bd993e992";
    std::string house_path = "../data/house/" + scene_id + "/house.json";
    std::ifstream house_stream(house_path);
    json j;
    house_stream >> j;

    basic_json levels = j["levels"];
    std::vector<Object> objects;
    for (auto &level : levels) {
        basic_json nodes = level["nodes"];
        for (auto &node: nodes) {
            if (node["valid"] == 1) {
                Object o(node);
                objects.push_back(o);
            }
        }
    }

    for (auto &obj_pair : cartesian_product(objects, objects)) {
        Object obj_a = obj_pair.first;
        Object obj_b = obj_pair.second;
        std::string rel = obj_a.relation_to(obj_b);
    }

    return 0;
}


