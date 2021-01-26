//
// Created by tim on 25.01.21.
//

#include "SUNCTransformer.h"

#include <fstream>
#include "json.hpp"
#include "cpptoml.h"

void SUNCTransformer::transform(const std::string& path) {
    // path is path to house.json
    std::ifstream house_stream(path);
    nlohmann::json j;
    house_stream >> j;

    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();

    auto table = cpptoml::make_table();
    table->insert("test", 2.0);

    root->insert("Dataset", table);

    std::cout << (*root);

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
