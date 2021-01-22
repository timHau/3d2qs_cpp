#include <iostream>
#include <fstream>
#include "json.hpp"
#include "Object.h"

using nlohmann::json;
using nlohmann::basic_json;

int main() {
    // load house.json
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
