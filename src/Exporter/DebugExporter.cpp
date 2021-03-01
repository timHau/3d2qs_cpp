//
// Created by tau on 29.01.21.
//

#include "DebugExporter.h"

/*
 * read .toml file and write them into an .ply file. Useful for debugging
 */
void DebugExporter::to_ply(const fs::path& input_dir)
{

	if (!fs::exists(input_dir))
		throw std::invalid_argument("input dir does not exists");

	for (const auto& config : fs::directory_iterator(input_dir))
	{
		const std::string toml_file = config.path().filename();
		const fs::path toml_path = input_dir / toml_file;
		try
		{
			fs::path room_id = toml_file;
			room_id.replace_extension("");
			handle_toml(toml_path, room_id, input_dir);
		}
		catch (cpptoml::parse_exception& e)
		{
			std::cerr << e.what();
		}
	}
}

void DebugExporter::handle_toml(const fs::path& toml_path, const std::string& room_id, const fs::path& input_dir)
{
	auto config = cpptoml::parse_file(toml_path);

	// every objects bounding box is written to its own .ply file
	for (const auto& obj : *config->get_table_array("object"))
	{
		std::string label = *(obj->get_qualified_as<std::string>("label"));
		std::string id = *(obj->get_qualified_as<std::string>("id"));
		auto bbox = *(obj->get_array_of<cpptoml::array>("bbox"));

		happly::PLYData plyOut;
		std::vector<std::array<double, 3>> vertices_bbox;
		std::vector<std::vector<int>> faces_bbox = {
				{ 0, 1, 2, 3 },
				{ 3, 2, 6, 7 },
				{ 1, 5, 6, 2 },
				{ 4, 5, 1, 0 },
				{ 4, 0, 3, 7 },
				{ 5, 4, 7, 6 },
		};

		for (int i = 0; i < 8; ++i)
		{
			std::vector<double> empty; // default option
			std::vector<double> point = bbox[i]->get_array_of<double>().value_or(empty);

			if (!point.empty())
			{
				std::array<double, 3> vertex_pos = { point[0], point[1], point[2] };
				vertices_bbox.emplace_back(vertex_pos);
			}
		}

		const fs::path debug_path = input_dir / ".." / "debug";
		if (!fs::exists(debug_path))
			fs::create_directory(debug_path);

		const fs::path dir_path = debug_path / room_id;
		if (!fs::exists(dir_path))
			fs::create_directory(dir_path);

		const std::string filename = "/" + id + "_" + label + "_bbox" + ".ply";
		const std::string output_path = dir_path.string() + filename;

		plyOut.addVertexPositions(vertices_bbox);
		plyOut.addFaceIndices(faces_bbox);
		plyOut.write(output_path, happly::DataFormat::ASCII);
		std::cout << "wrote " << output_path << std::endl;
	}
}
