#include "MatterportTransformer.h"

struct FACE {
    int seg_ind; // index of the segment that this face is contained in
    std::vector<std::vector<double>> vertices; // 3 vertices that build the face
};


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
    std::vector<double> vert_z = ply_file.getElement("vertex").getProperty<double>("z");
    std::vector<std::vector<int>> vertex_indices = ply_file.getElement("face").getListProperty<int>("vertex_indices");

    // every entry in the array 'segIndices' corresponds to one face
    // the i-th face is contained in the segment with id segIndices[i]
    auto fseg_indices = fseg_json["segIndices"];
    std::vector<FACE> faces_per_region;
    for (int i = 0; i < fseg_indices.size(); ++i) {
        // the value 'vertex_indices' inside the .ply corresponds to the index of 3 vertices inside the face
        std::vector<std::vector<double>> vertices;
        for (const int idx : vertex_indices[i]) {
            vertices.push_back({vert_x[idx], vert_y[idx], vert_z[idx]});
        }
        auto seg_ind = fseg_indices[i];
        FACE f{seg_ind, vertices};
        faces_per_region.emplace_back(f);
    }

    // prepare .toml
    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
    auto object_table_array = cpptoml::make_table_array();

    for (const auto &seg_group : semseg_json["segGroups"]) {
        // get the faces (vector of vertices) of each object
        std::vector<FACE> faces_per_object;
        std::vector<int> segments = seg_group["segments"];
        for (const int seg_id : segments) {
            for (const FACE &f : faces_per_region) {
                if (f.seg_ind == seg_id) {
                    faces_per_object.emplace_back(f);
                }
            }
        }

        // calculate the bounding box
        double min_x = std::numeric_limits<double>::infinity(), max_x = -std::numeric_limits<double>::infinity();
        double min_y = std::numeric_limits<double>::infinity(), max_y = -std::numeric_limits<double>::infinity();
        double min_z = std::numeric_limits<double>::infinity(), max_z = -std::numeric_limits<double>::infinity();
        for (const FACE &f : faces_per_object) {
            for (const std::vector<double> &vertex : f.vertices) {
                if (vertex[0] < min_x) { min_x = vertex[0]; }
                if (vertex[0] > max_x) { max_x = vertex[0]; }
                if (vertex[1] < min_y) { min_y = vertex[1]; }
                if (vertex[1] > max_y) { max_y = vertex[1]; }
                if (vertex[2] < min_z) { min_z = vertex[2]; }
                if (vertex[2] > max_z) { max_z = vertex[2]; }
            }
        }

        std::vector<std::vector<double>> bbox = {
                {min_x, min_y, max_z},  // vorne, unten, links
                {max_x, min_y, max_z},  // vorne, unten, rechts
                {max_x, max_y, max_z},  // vorne, oben, rechts
                {min_x, max_y, max_z},  // vorne, oben, links
                {min_x, min_y, min_z},  // hinten, unten, links
                {max_x, min_y, min_z},  // hinten, unten, rechts
                {max_x, max_y, min_z},  // hinten, oben, rechts
                {min_x, max_y, min_z},  // hinten, oben, links
        };

        auto bbox_array = cpptoml::make_array();
        for (const std::vector<double> &v : bbox) {
            auto row_array = cpptoml::make_array();
            for (auto v_i : v) {
                row_array->push_back(v_i);
            }
            bbox_array->push_back(row_array);
        }

        auto object_table = cpptoml::make_table();
        object_table->insert("bbox", bbox_array);
        object_table->insert("id", seg_group["id"]);
        object_table->insert("label", seg_group["label"]);
        object_table_array->push_back(object_table);
    }

    root->insert("object", object_table_array);

    auto meta_table = cpptoml::make_table();
    meta_table->insert("name", "Matterport3d");
    root->insert("dataset", meta_table);

    std::ofstream output;
    output.open("../data/matterport.toml");
    output << (*root);
    output.close();
    std::cout << "wrote matterport.toml" << std::endl;
}
