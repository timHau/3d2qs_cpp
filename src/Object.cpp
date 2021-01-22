//
// Created by tim on 22.01.21.
//

#include "Object.h"
#include <iostream>

Object::Object(nlohmann::json &node) {
    std::cout << node["bbox"]["max"] << std::endl;
}
