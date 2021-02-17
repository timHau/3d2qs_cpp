#include "SUNCTransformer.h"

void SUNCTransformer::transform(const std::string& path)
{
	std::cout << "start parsing sunc data" << std::endl;

	const fs::path sunc_path(path);
	const fs::path house_path = sunc_path / "house/";
	const fs::path object_path = sunc_path / "object/";
	const fs::path config_path = sunc_path / "config/";

	for (const auto& room : fs::directory_iterator(house_path))
	{
		nlohmann::json json_data;
		const std::string room_id = room.path().filename();
		try
		{
			const fs::path house_json_path = house_path / room_id / "house.json";
			std::ifstream house_stream(house_json_path);
			house_stream >> json_data;

			const fs::path output_path = config_path / (room_id + ".toml");
			handle_room(json_data, output_path);
		}
		catch (nlohmann::json::parse_error& e)
		{
			std::cerr << room_id << std::endl;
			std::cerr << e.what();
		}
	}
}

void SUNCTransformer::handle_room(const nlohmann::json& json_data, const fs::path& output_path)
{

	std::shared_ptr<cpptoml::table> root = cpptoml::make_table();
	auto object_table_array = cpptoml::make_table_array();

	nlohmann::basic_json levels = json_data["levels"];
	for (auto& level : levels)
	{
		nlohmann::basic_json nodes = level["nodes"];
		for (auto& node : nodes)
		{
			if (node["valid"] == 1)
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

				auto bbox_array = cpptoml::make_array();
				for (const Eigen::Vector3d& v : bbox)
				{
					// rotate the boxes to fit to blender coordinate system
					Eigen::Matrix3d m;
					m = Eigen::AngleAxisd(M_PI_2, Eigen::Vector3d::UnitX());
					Eigen::Vector3d rotated = m * v;

					auto row_array = cpptoml::make_array();
					for (auto v_i : rotated)
					{
						row_array->push_back(v_i);
					}
					bbox_array->push_back(row_array);
				}

				// for now only handle objects
				if (node["type"] == "Object" && node["valid"] == 1)
				{
					// add transformation
					auto transform = cpptoml::make_array();
					for (double t_i : node["transform"]) {
						transform->push_back(t_i);
					}

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
	}

	root->insert("object", object_table_array);

	auto meta_table = cpptoml::make_table();
	meta_table->insert("name", "SUNC");
	root->insert("dataset", meta_table);

	std::ofstream output;
	output.open(output_path);
	output << (*root);
	output.close();
	std::cout << "wrote: " << output_path << std::endl;
}
