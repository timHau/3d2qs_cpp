#include "MatterportTransformer.h"

struct Face {
    int seg_ind; // index of the segment that this face is contained in
    std::vector<std::vector<double>> vertices; // 3 vertices that build the face
};


void MatterportTransformer::transform(const std::string &path) {
    std::cout << "start parsing matterport3d data" << std::endl;

    std::string base_output_path = "../data/matterport3d/config/";

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
    std::vector<Face> faces_per_region;
    for (int i = 0; i < fseg_indices.size(); ++i) {
        // the value 'vertex_indices' inside the .ply corresponds to the index of 3 vertices inside the face
        std::vector<std::vector<double>> vertices;
        for (const int idx : vertex_indices[i]) {
            vertices.push_back({vert_x[idx], vert_y[idx], vert_z[idx]});
        }
        auto seg_ind = fseg_indices[i];
        Face f{seg_ind, vertices};
        faces_per_region.emplace_back(f);
    }

    // prepare .toml
    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
    auto object_table_array = cpptoml::make_table_array();

    for (const auto &seg_group : semseg_json["segGroups"]) {
        // get the faces (vector of vertices) of each object
        std::vector<Face> faces_per_object;
        std::vector<int> segments = seg_group["segments"];
        for (const int seg_id : segments) {
            for (const Face &f : faces_per_region) {
                if (f.seg_ind == seg_id) {
                    faces_per_object.emplace_back(f);
                }
            }
        }

        std::vector<double> vertices_per_object = {};
        std::vector<int> face_indices = {};

        for (const Face &f : faces_per_object) {
            for (auto &vertex : f.vertices) {
                vertices_per_object.push_back(vertex[0]);
                vertices_per_object.push_back(vertex[1]);
                vertices_per_object.push_back(vertex[2]);
            }
        }


        /*
        auto bbox_array = cpptoml::make_array();
        for (const std::vector<double> &v : bbox) {
            auto row_array = cpptoml::make_array();
            for (auto v_i : v) {
                row_array->push_back(v_i);
            }
            bbox_array->push_back(row_array);
        }
         */

        std::string label = seg_group["label"];
        std::string id = std::to_string((int) seg_group["id"]);
        std::string obj_name = id + "_" + label + ".obj";

        // construct matrices of vertices and faces for libigl
        auto num_rows_vert = vertices_per_object.size() / 3;
        auto num_rows_face = face_indices.size() / 3;
        Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            V(vertices_per_object.data(), num_rows_vert, 3);
        Eigen::Map<Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
            F(face_indices.data(), num_rows_face, 3);
        // write every object as a .obj file
        igl::writeOBJ(base_output_path + obj_name, V, F);
        std::cout << "writing " << obj_name << std::endl;

        auto object_table = cpptoml::make_table();
//        object_table->insert("bbox", bbox_array);
        object_table->insert("id", id);
        object_table->insert("label", label);
        object_table_array->push_back(object_table);
    }

    root->insert("object", object_table_array);

    auto meta_table = cpptoml::make_table();
    meta_table->insert("name", "Matterport3d");
    root->insert("dataset", meta_table);

    std::ofstream output;
    output.open(base_output_path + "matterport3d.toml");
    output << (*root);
    output.close();
    std::cout << "wrote matterport3d.toml" << std::endl;
}
