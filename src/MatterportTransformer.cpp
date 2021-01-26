//
// Created by tim on 26.01.21.
//

#include "MatterportTransformer.h"


void MatterportTransformer::transform(const std::string &path) {
    std::ifstream semseg_stream(path + "region17.semseg.json");
    nlohmann::json semseg_json;
    semseg_stream >> semseg_json;

    std::ifstream fseg_stream(path + "region17.fsegs.json");
    nlohmann::json fseg_json;
    fseg_stream >> fseg_json;

    happly::PLYData ply_file(path + "region17.ply");
    std::vector<double> vert_x = ply_file.getElement("vertex").getProperty<double>("x");
    std::vector<double> vert_y = ply_file.getElement("vertex").getProperty<double>("y");
    std::vector<double> vert_z = ply_file.getElement("vertex").getProperty<double>("y");
    std::vector<std::vector<int>> vertex_indices = ply_file.getElement("face").getListProperty<int>("vertex_indices");

    // every entry in the array 'segIndices' corresponds to one face
    // the i-th face is contained in the segment with id segIndices[i]
    auto fseg_indices = fseg_json["segIndices"];
    std::vector<std::vector<double>> faces;
    for (int i = 0; i < fseg_indices.size(); ++i) {
        auto seg_ind = fseg_indices[i];
        // the value 'vertex_indices' inside the .ply corresponds to the index of 3 vertices inside the face
        for (const int & idx : vertex_indices[i]) {
            std::vector<double> face = { vert_x[idx], vert_y[idx], vert_y[idx] };
            faces.emplace_back(face);
        }
    }


    // prepare .toml
    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
    auto object_table_array = cpptoml::make_table_array();

    for (const auto &seg_group : semseg_json["segGroups"]) {
        std::cout << seg_group << std::endl;
        auto object_table = cpptoml::make_table();
//        object_table->insert("bbox", bbox_array);
        object_table->insert("label", seg_group["label"]);
        object_table_array->push_back(object_table);

        // std::cout << seg_group << std::endl;
    }

    root->insert("object", object_table_array);

    auto meta_table = cpptoml::make_table();
    meta_table->insert("name", "Matterport3d");
    root->insert("dataset", meta_table);

    std::cout << (*root);
}
