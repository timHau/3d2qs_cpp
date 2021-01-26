#include "SUNCTransformer.h"

void SUNCTransformer::transform(const std::string &path) {
    // path is path to house.json
    std::ifstream house_stream(path);
    nlohmann::json j;
    house_stream >> j;

    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
    auto object_table_array = cpptoml::make_table_array();

    nlohmann::basic_json levels = j["levels"];
    for (auto &level : levels) {
        nlohmann::basic_json nodes = level["nodes"];
        for (auto &node : nodes) {
            if (node["valid"] == 1) {
                std::vector<double> max = node["bbox"]["max"];
                std::vector<double> min = node["bbox"]["min"];
                std::vector<Eigen::Vector3d> bbox = {
                        Eigen::Vector3d(min[0], min[1], max[2]),   // vorne, unten, links
                        Eigen::Vector3d(max[0], min[1], max[2]),   // vorne, unten, rechts
                        Eigen::Vector3d(max[0], max[1], max[2]),   // vorne, oben, rechts
                        Eigen::Vector3d(min[0], max[1], max[2]),   // vorne, oben, links
                        Eigen::Vector3d(min[0], min[1], min[2]),   // hinten, unten, links
                        Eigen::Vector3d(max[0], min[1], min[2]),   // hinten, unten, rechts
                        Eigen::Vector3d(max[0], max[1], min[2]),   // hinten, oben, rechts
                        Eigen::Vector3d(min[0], max[1], min[2])    // hinten, oben, links
                };

                if (node.contains("transform")) {
                    // SUNC data is centered at the origin
                    // so we have to transform it back to obtain the true intersections
                    std::vector<double> t = node["transform"];
                    // transform is column major
                    Eigen::Matrix4d transform;
                    transform << t[0],  t[4],  t[8],  t[12],
                                 t[1],  t[5],  t[9],  t[13],
                                 t[2],  t[6],  t[10], t[14],
                                 t[3],  t[7],  t[11], t[15];

                    std::vector<Eigen::Vector3d> transformed_bbox;
                    // convert bbox vectors into homogenous coordinates, transform there and convert back
                    for (auto & v : bbox) {
                        Eigen::Vector4d hom;
                        hom << v, 1.0;
                        hom = transform*hom;
                        transformed_bbox.emplace_back(hom.head<3>());
                    }

                    bbox = transformed_bbox;
                }

                auto bbox_array = cpptoml::make_array();
                for (const Eigen::Vector3d &v : bbox) {
                    auto row_array = cpptoml::make_array();
                    for (auto v_i : v) {
                        row_array->push_back(v_i);
                    }
                    bbox_array->push_back(row_array);
                }

                // parse nodes to one toml file
                auto object_table = cpptoml::make_table();
                object_table->insert("bbox", bbox_array);
                // TODO replace modelId by true label
                object_table->insert("label", node["modelId"]);
                object_table_array->push_back(object_table);
            }
        }
    }

    root->insert("object", object_table_array);

    auto meta_table = cpptoml::make_table();
    meta_table->insert("name", "SUNC");
    root->insert("dataset", meta_table);

    std::ofstream output;
    output.open("../data/SUNC.toml");
    output << (*root);
    output.close();
    std::cout << "wrote SUNC.toml";
}
