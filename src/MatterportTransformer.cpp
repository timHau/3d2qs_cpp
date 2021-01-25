//
// Created by tim on 25.01.21.
//

#include "MatterportTransformer.h"

#include <fstream>
#include "json.hpp"

void MatterportTransformer::transform(const std::string& path) {
    // path is path to house.json
    std::ifstream house_stream(path);
    nlohmann::json j;
    house_stream >> j;
    nlohmann::basic_json levels = j["levels"];
    for (auto &level : levels) {
        nlohmann::basic_json nodes = level["nodes"];
        for (auto &node : nodes) {
            if (node["valid"] == 1) {
                // parse nodes to one toml file
            }
        }
    }
}
