#include <iostream>
#include "cpptoml.h"
#include "Object.h"
#include "Utils.h"

#include "Transformer/SUNCTransformer.h"
#include "Transformer/MatterportTransformer.h"
#include "Exporter.h"

int main() {
    // TODO make os independent
    // load house.json

    SUNCTransformer::transform("../data/sunc/");
    // MatterportTransformer::transform("../data/matterport3d/region_segmentations/");

    auto config = cpptoml::parse_file("../data/matterport3d/config/matterport3d.toml");
    auto val = config->get_qualified_as<std::string>("dataset.name");
    std::vector<Object> objects;
    for (const auto &obj : *config->get_table_array("object")) {
        Object object(obj);
        objects.emplace_back(object);
    }

    Exporter::to_ply("../data/sunc/config/");

    /*
    for (auto &obj_pair : utils::cartesian_product(objects, objects)) {
        Object obj_a = obj_pair.first;
        Object obj_b = obj_pair.second;
        std::string rel = obj_a.relation_to(obj_b);
        std::cout << *obj_a.get_label() << " with id: " << *obj_a.get_id() << " is in " << rel << " relation with "
                  << *obj_b.get_label() << " with id: " << *obj_b.get_id() << std::endl;
    }

    /*
    Object obj_a = objects[77];
    Object obj_b = objects[76];
    std::string rel = obj_a.relation_to(obj_b);
    std::cout << *obj_a.get_label() << " with id: " << *obj_a.get_id() << " is in " << rel << " relation with "
              << *obj_b.get_label() << " with id: " << *obj_b.get_id() << std::endl;
     */

    return 0;
}
