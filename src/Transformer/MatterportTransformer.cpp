#include "MatterportTransformer.h"

void MatterportTransformer::transform(const std::string& path)
{
	const fs::path matterport_path(path);
	const std::string house_name = "1pXnuDYAj8r";
	const fs::path house_path = matterport_path / "house_segmentations";
	std::cout << "start parsing " << house_name << " (matterport)" << std::endl;
	auto house = read_house_file(house_path, house_name);
	handle_house(house, matterport_path, house_name);
}

void MatterportTransformer::handle_house(
		std::map<std::string, std::vector<std::string>>& house,
		const fs::path& matterport_path,
		const std::string& house_name
)
{
	// for every region in the house
	for (int i = 0; i < house["region_indices"].size(); ++i)
	{
		// get all objects in that region
		std::string region_id = std::to_string(i);
		std::cout << "get all objects for region" << region_id << std::endl;
		auto objects_per_region = get_objects_per_region(house, region_id);

		// create a folder inside the config folder and create a .toml for every region
		const std::string region_name = "region" + region_id;
		const fs::path config_dir = matterport_path / "config" / (house_name + "_" + region_name);
		if (!fs::exists(config_dir))
			fs::create_directory(config_dir);
		write_objects_to_toml(objects_per_region, config_dir, region_name);
	}
}

void MatterportTransformer::write_objects_to_toml(
		std::vector<Obj>& objects,
		const fs::path& config_dir,
		const std::string& region_name
		)
{
	std::shared_ptr<cpptoml::table> region_root = cpptoml::make_table();
	auto object_table_array = cpptoml::make_table_array();

	for (Obj& obj : objects)
	{
		auto object_table = cpptoml::make_table();
//		object_table->insert("bbox", bbox_array);
		object_table->insert("id", obj.object_index);
		object_table->insert("label", obj.catergory_name);
		object_table_array->push_back(object_table);
	}

	region_root->insert("object", object_table_array);

	const fs::path output_path = config_dir / (region_name + ".toml");
	std::ofstream output;
	output.open(output_path);
	output << (*region_root);
	output.close();
	std::cout << "wrote: " << output_path << std::endl;
}

std::vector<Obj> MatterportTransformer::get_objects_per_region(
		std::map<std::string, std::vector<std::string>>& house,
		std::string& region_id
)
{
	std::vector<Obj> objects_per_region;
	for (const auto& obj_line : house["object_indices"])
	{
		auto splited_obj = split_line(obj_line);
		if (region_id == splited_obj[2])
		{
			auto category_index = splited_obj[3];
			auto object_index = splited_obj[1];
			auto obb_str = std::vector<std::string>(splited_obj.begin() + 4, splited_obj.begin() + 15);

			std::vector<double> obb_d(obb_str.size());
			std::transform(obb_str.begin(), obb_str.end(), obb_d.begin(), [](const std::string& val) { return std::stod(val); });

			Obj o{ category_index, object_index, obb_d };
			objects_per_region.push_back(o);
		}
	}

	for (Obj& o : objects_per_region)
	{
		// find the right category matching category_index
		for (const auto& category_line : house["category_indices"])
		{
			auto splited_cat = split_line(category_line);
			if (o.category_index == splited_cat[1])
				o.catergory_name = splited_cat[3];
		}

		std::vector<int> segment_indices;
		// find all segment_indices that are part of that object
		for (const auto& segment_line : house["segment_indices"])
		{
			auto splited_seg = split_line(segment_line);
			if (o.object_index == splited_seg[2])
				segment_indices.push_back(std::stoi(splited_seg[1]));
		}
		o.segments_indices = segment_indices;
	}
	return objects_per_region;
}

std::map<std::string, std::vector<std::string>> MatterportTransformer::read_house_file(
		const fs::path& house_path,
		const std::string& house_name
)
{
	std::vector<std::string> level_indices;
	std::vector<std::string> region_indices;
	std::vector<std::string> portal_indices;
	std::vector<std::string> surface_indices;
	std::vector<std::string> vertex_indices;
	std::vector<std::string> category_indices;
	std::vector<std::string> object_indices;
	std::vector<std::string> segment_indices;

	std::ifstream house_file(house_path / (house_name + ".house"));
	std::string line;
	while (std::getline(house_file, line))
	{
		if (line.at(0) == 'L')
			level_indices.push_back(line);
		if (line.at(0) == 'R')
			region_indices.push_back(line);
		if (line.at(0) == 'P')
			portal_indices.push_back(line);
		if (line.at(0) == 'S')
			surface_indices.push_back(line);
		if (line.at(0) == 'V')
			vertex_indices.push_back(line);
		if (line.at(0) == 'C')
			category_indices.push_back(line);
		if (line.at(0) == 'O')
			object_indices.push_back(line);
		if (line.at(0) == 'S')
			segment_indices.push_back(line);
	}

	std::map<std::string, std::vector<std::string>> res;
	res["level_indices"] = level_indices;
	res["region_indices"] = region_indices;
	res["portal_indices"] = portal_indices;
	res["surface_indices"] = surface_indices;
	res["vertex_indices"] = vertex_indices;
	res["category_indices"] = category_indices;
	res["object_indices"] = object_indices;
	res["segment_indices"] = segment_indices;

	return res;
}

std::vector<std::string> MatterportTransformer::split_line(const std::string& line)
{
	std::vector<std::string> res;
	std::istringstream iss(line);
	std::copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			std::back_inserter(res));
	return res;
}

void MatterportTransformer::write_as_ply(
		const fs::path& out_path,
		std::vector<double>& vert_x_out,
		std::vector<double>& vert_y_out,
		std::vector<double>& vert_z_out,
		std::vector<std::vector<int>>& vert_indices_out
)
{
	// write each object as a .ply
	happly::PLYData objectPly;
	objectPly.addElement("vertex", vert_x_out.size());
	objectPly.addElement("face", vert_indices_out.size());
	objectPly.getElement("vertex").addProperty<double>("x", vert_x_out);
	objectPly.getElement("vertex").addProperty<double>("y", vert_y_out);
	objectPly.getElement("vertex").addProperty<double>("z", vert_z_out);
	objectPly.getElement("face").addListProperty<int>("vertex_indices", vert_indices_out);
	objectPly.write(out_path);
	std::cout << "wrote: " << out_path << std::endl;
}

std::vector<std::string> MatterportTransformer::get_column_tsv(const std::string& file_name, int column)
{
	std::vector<std::string> res = {};

	std::ifstream file(file_name);
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string token;
		std::vector<std::string> tokens;
		while (std::getline(iss, token, '\t'))
			tokens.push_back(token);

		if (column < tokens.size())
			res.push_back(tokens[column]);
	}

	// we dont need the heading of the column
	res.erase(res.begin());

	return res;
}


