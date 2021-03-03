#include "SUNCTransformer.h"
#define TINYOBJLOADER_IMPLEMENTATION // only define once
#include "tiny_obj_loader.h"

void SUNCTransformer::transform(const std::string& path)
{
	transform(path, false);
}

void SUNCTransformer::transform(const std::string& path, bool debug)
{
	std::cout << "start parsing sunc data" << std::endl;

	const fs::path sunc_path(path);
	const fs::path house_path = sunc_path / "house";
	const fs::path object_path = sunc_path / "object";
	const fs::path config_path = sunc_path / "config";
	const fs::path debug_path = sunc_path / "debug";
	if (!fs::exists(debug_path))
		fs::create_directory(debug_path);

	for (const auto& room : fs::directory_iterator(house_path))
	{
		nlohmann::json json_data;
		const std::string room_id = room.path().filename();
		try
		{
			const fs::path house_json_path = house_path / room_id / "house.json";
			std::ifstream house_stream(house_json_path);
			house_stream >> json_data;

			const fs::path objects_room_path = debug_path / room_id;
			if (!fs::exists(objects_room_path))
				fs::create_directory(objects_room_path);

			const fs::path output_path = config_path / (room_id + ".toml");
			handle_room(json_data, room_id, object_path, objects_room_path, output_path, debug);
		}
		catch (nlohmann::json::parse_error& e)
		{
			std::cerr << room_id << std::endl;
			std::cerr << e.what();
		}
	}
}

void SUNCTransformer::handle_room(
		const nlohmann::json& json_data,
		const std::string& room_id,
		const fs::path& object_path,
		const fs::path& objects_path,
		const fs::path& output_path,
		bool debug)
{
	std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
	auto object_table_array = cpptoml::make_table_array();

	nlohmann::basic_json levels = json_data["levels"];
	for (auto& level : levels)
	{
		nlohmann::basic_json nodes = level["nodes"];
		for (auto& node : nodes)
		{
			// first restrict ourself to valid objects, other possible types are room and ground, maybe more
			if (node["valid"] == 1 && node["type"] == "Object")
			{
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

				Eigen::Matrix3d rot;
				rot = Eigen::AngleAxisd(M_PI_2, Eigen::Vector3d::UnitX());

				auto bbox_array = cpptoml::make_array();
				for (const Eigen::Vector3d& v : bbox)
				{
					// rotate the boxes to fit to blender coordinate system
					// the bounding boxes are already transformed
					Eigen::Vector3d rotated = rot * v;
					auto row_array = cpptoml::make_array();
					for (auto v_i : rotated)
						row_array->push_back(v_i);
					bbox_array->push_back(row_array);
				}

				// need to transform the objects
				Eigen::Matrix4d t;
				t << node["transform"][0], node["transform"][1], node["transform"][2], node["transform"][3],
					 node["transform"][4], node["transform"][5], node["transform"][6], node["transform"][7],
					 node["transform"][8], node["transform"][9], node["transform"][10], node["transform"][11],
					 node["transform"][12], node["transform"][13], node["transform"][14], node["transform"][15];

				const std::string model_id = node["modelId"];
				fs::path obj_path = object_path / model_id / (model_id + ".obj");
				auto object = handle_object(obj_path, room_id, t, rot);
				if (debug)
					write_object_to_ply(object, objects_path);

				// add transformation
				auto transform = cpptoml::make_array();
				for (double t_i : node["transform"])
					transform->push_back(t_i);

				// parse nodes to one toml file
				auto object_table = cpptoml::make_table();
				object_table->insert("bbox", bbox_array);
				object_table->insert("id", node["modelId"]);
				object_table->insert("transform", transform);
				// TODO replace modelId by true label
				object_table->insert("label", node["modelId"]);
				object_table_array->push_back(object_table);
			}
		}
	}

	root->insert("object", object_table_array);

	auto meta_table = cpptoml::make_table();
	meta_table->insert("name", "SUNC_" + room_id);
	root->insert("dataset", meta_table);

	std::ofstream output;
	output.open(output_path);
	output << (*root);
	output.close();
	std::cout << "wrote: " << output_path << std::endl;
}

/*
 * Get the .obj object
 */
SuncObject SUNCTransformer::handle_object(
		fs::path& obj_path,
		const std::string& room_id,
		Eigen::Matrix4d& transform,
		Eigen::Matrix3d& rot)
{
	std::vector<Eigen::Vector3d> vertices;

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./";
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(obj_path.c_str(), reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	// Loop over shapes
	for (const auto & shape : shapes) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
			int fv = shape.mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
				tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
				tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];

				Eigen::Vector4d vec_h(vx, vy, vz, 1.0);
				Eigen::Vector4d t_vec = transform.transpose() * vec_h;
				Eigen::Vector3d vec_r(t_vec[0]/t_vec[3], t_vec[1]/t_vec[3], t_vec[2]/t_vec[3]);
				Eigen::Vector3d rot_vec = rot * vec_r;
				vertices.push_back(rot_vec);
			}
			index_offset += fv;
		}
	}

	std::string id = obj_path.stem();
	SuncObject object{id, room_id, vertices};
	return object;
}

void SUNCTransformer::write_object_to_ply(SuncObject& object, const fs::path& objects_path)
{
	std::vector<double> vert_x_out;
	std::vector<double> vert_y_out;
	std::vector<double> vert_z_out;
	std::vector<std::vector<int>> vert_indices_out;

	for (int i = 0; i < object.vertices.size(); i += 3)
	{
		for (int j = 0; j < 3; ++j)
		{
			vert_x_out.push_back(object.vertices[i + j][0]);
			vert_y_out.push_back(object.vertices[i + j][1]);
			vert_z_out.push_back(object.vertices[i + j][2]);
		}

		std::vector<int> vert_index{ i, i + 1, i + 2 };
		vert_indices_out.push_back(vert_index);
	}

	const fs::path obj_path = objects_path / ( object.id + ".ply");

	// write each object as a .ply
	happly::PLYData objectPly;
	objectPly.addElement("vertex", vert_x_out.size());
	objectPly.addElement("face", vert_indices_out.size());
	objectPly.getElement("vertex").addProperty<double>("x", vert_x_out);
	objectPly.getElement("vertex").addProperty<double>("y", vert_y_out);
	objectPly.getElement("vertex").addProperty<double>("z", vert_z_out);
	objectPly.getElement("face").addListProperty<int>("vertex_indices", vert_indices_out);
	objectPly.write(obj_path);
	std::cout << "wrote: " << obj_path << std::endl;
}
