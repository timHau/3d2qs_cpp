#include <iostream>
#include "cpptoml.h"
#include "Object.h"
#include <filesystem>

#include "Transformer/SUNCTransformer.h"
#include "Transformer/MatterportTransformer.h"

#include "Exporter/DebugExporter.h"
#include "Exporter/XmlExporter.h"

namespace fs = std::filesystem;


int main() {
    SUNCTransformer::transform("../data/sunc/");
    MatterportTransformer::transform("../data/matterport3d/region_segmentations/");

    for (auto & f : fs::directory_iterator("../data")) {
        std::cout << f.path() << std::endl;
    }

    auto config = cpptoml::parse_file("../data/matterport3d/config/matterport3d.toml");
    auto val = config->get_qualified_as<std::string>("dataset.name");
    std::vector<Object> objects_matterport;
    for (const auto &obj : *config->get_table_array("object")) {
        Object object{obj};
        objects_matterport.emplace_back(object);
    }

    fs::path xml_output_path_matterport("../data/matterport3d/xml/matterport3d.xml");
    XmlExporter::to_xml(xml_output_path_matterport, objects_matterport);

    DebugExporter::to_ply("../data/sunc/config/");
    DebugExporter::to_ply("../data/matterport3d/config/");

    /*
    Object obj_a = objects[77];
    Object obj_b = objects[76];
    std::string rel = obj_a.relation_to(obj_b);
    std::cout << *obj_a.get_label() << " with id: " << *obj_a.get_id() << " is in " << rel << " relation with "
              << *obj_b.get_label() << " with id: " << *obj_b.get_id() << std::endl;
     */

    return 0;
}
