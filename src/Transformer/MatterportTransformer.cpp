#include "MatterportTransformer.h"

void MatterportTransformer::transform(const std::string& path)
{
	const fs::path matterport_path(path);
	const std::string house_name = "1pXnuDYAj8r";
	const fs::path house_path = matterport_path / "house_segmentations";
	std::cout << "start parsing " << house_name << " (matterport)" << std::endl;
	auto house = read_house_file(house_path, house_name);
	handle_house(house, matterport_path, house_path, house_name);
}

void MatterportTransformer::handle_house(
		std::map<std::string, std::vector<std::string>>& house,
		const fs::path& matterport_path,
		const fs::path& house_path,
		const std::string& house_name
)
{
	// for every region in the house
	for (int i = 0; i < house["region_indices"].size(); ++i)
	{
		// get all objects in that region
		std::string region_id = std::to_string(i);
		std::cout << "get all objects for region" << region_id << std::endl;
		auto objects_per_region = get_objects_per_region(house, house_path, house_name, matterport_path, region_id);


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
		// create transform matrix from obb
		// ref: https://stackoverflow.com/questions/53227533/how-to-find-out-the-rotation-matrix-for-the-oriented-bounding-box
		Eigen::Matrix<double, 4, 4, Eigen::ColMajor> t;
		t << obj.normalized_axes[0], obj.normalized_axes[1], obj.normalized_axes[2], 0.0,
			 obj.normalized_axes[3], obj.normalized_axes[4], obj.normalized_axes[5], 0.0,
			 obj.normalized_axes[5], obj.normalized_axes[6], obj.normalized_axes[7], 0.0,
			 obj.centroid[0],        obj.centroid[1],        obj.centroid[2],        1.0;
		auto transform = t.inverse();

		auto transform_array = cpptoml::make_array();
		for (int i = 0; i < transform.size(); ++i)
			transform_array->push_back(*(t.data() + i));

		auto bbox_array = cpptoml::make_array();
		for (const std::vector<double>& v : obj.bbox)
		{
			auto row_array = cpptoml::make_array();
			for (auto v_i : v)
				row_array->push_back(v_i);
			bbox_array->push_back(row_array);
		}

		object_table->insert("bbox", bbox_array);
		object_table->insert("id", obj.object_index);
		object_table->insert("label", obj.catergory_name);
		object_table->insert("transform", transform_array);
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
		const fs::path& house_path,
		const std::string& house_name,
		const fs::path& matterport_path,
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

	std::ifstream semseg_stream(house_path / (house_name + ".semseg.json"));
	nlohmann::json semseg_json;
	semseg_stream >> semseg_json;
	auto seg_groups = semseg_json["segGroups"];

	const fs::path region_path = matterport_path / "region_segmentations";
	std::ifstream fseg_stream(region_path / ("region" + region_id + ".fsegs.json"));
	nlohmann::json fseg_json;
	fseg_stream >> fseg_json;
	std::vector<int> fseg_indices = fseg_json["segIndices"];

	happly::PLYData ply_file(region_path / ("region" + region_id + ".ply"));
	std::vector<double> vert_x = ply_file.getElement("vertex").getProperty<double>("x");
	std::vector<double> vert_y = ply_file.getElement("vertex").getProperty<double>("y");
	std::vector<double> vert_z = ply_file.getElement("vertex").getProperty<double>("z");
	std::vector<std::vector<int>> vertex_indices = ply_file.getElement("face").getListProperty<int>("vertex_indices");

	// every entry in the array 'segIndices' corresponds to one face
	// the i-th face is contained in the segment with id segIndices[i]
	std::vector<Face> faces_per_region;
	for (int i = 0; i < fseg_indices.size(); ++i)
	{
		// the value 'vertex_indices' inside the .ply corresponds to the index of 3 vertices inside the face
		std::vector<std::vector<double>> vertices;
		for (const int idx : vertex_indices[i])
			vertices.push_back({ vert_x[idx], vert_y[idx], vert_z[idx] });
		auto seg_ind = fseg_indices[i];
		Face f{ seg_ind, vertices };
		faces_per_region.emplace_back(f);
	}

	for (Obj& obj : objects_per_region)
	{
		// find the right category matching category_index
		for (const auto& category_line : house["category_indices"])
		{
			auto splited_cat = split_line(category_line);
			if (obj.category_index == splited_cat[1])
				obj.catergory_name = splited_cat[3];
		}

		std::vector<int> segment_indices;
		// find all segment_indices that are part of that object
		for (const auto& segment_line : house["segment_indices"])
		{
			auto splited_seg = split_line(segment_line);
			if (obj.object_index == splited_seg[2])
				segment_indices.push_back(std::stoi(splited_seg[1]));
		}
		obj.segments_indices = segment_indices;

		// find centroid / axes length / dominant normal / normalized axes
		for (const auto& seg_group : seg_groups)
		{
			if (seg_group["id"] == std::stoi(obj.object_index))
			{
				std::vector<double> centroid = seg_group["obb"]["centroid"];
				std::vector<double> axes_length = seg_group["obb"]["axesLengths"];
				std::vector<double> dominant_normal = seg_group["obb"]["dominantNormal"];
				std::vector<double> normalized_axes = seg_group["obb"]["normalizedAxes"];
				obj.centroid = centroid;
				obj.axes_length = axes_length;
				obj.dominant_normal = dominant_normal;
				obj.normalized_axes = normalized_axes;
			}
		}



		// get the faces (vector of vertices) of each object
		std::vector<Face> faces_per_object;
		for (const int seg_id : obj.segments_indices)
		{
			for (const Face& f : faces_per_region)
			{
				if (f.seg_ind == seg_id)
					faces_per_object.emplace_back(f);
			}
		}

		// calculate the bounding box
		double min_x = std::numeric_limits<double>::infinity(), max_x = -std::numeric_limits<double>::infinity();
		double min_y = std::numeric_limits<double>::infinity(), max_y = -std::numeric_limits<double>::infinity();
		double min_z = std::numeric_limits<double>::infinity(), max_z = -std::numeric_limits<double>::infinity();
		for (const Face& f : faces_per_object)
		{
			for (const std::vector<double>& vertex : f.vertices)
			{
				if (vertex[0] < min_x) min_x = vertex[0];
				if (vertex[0] > max_x) max_x = vertex[0];
				if (vertex[1] < min_y) min_y = vertex[1];
				if (vertex[1] > max_y) max_y = vertex[1];
				if (vertex[2] < min_z) min_z = vertex[2];
				if (vertex[2] > max_z) max_z = vertex[2];
			}
		}

		std::vector<std::vector<double>> bbox = {
				{ min_x, min_y, max_z },  // vorne, unten, links
				{ max_x, min_y, max_z },  // vorne, unten, rechts
				{ max_x, max_y, max_z },  // vorne, oben, rechts
				{ min_x, max_y, max_z },  // vorne, oben, links
				{ min_x, min_y, min_z },  // hinten, unten, links
				{ max_x, min_y, min_z },  // hinten, unten, rechts
				{ max_x, max_y, min_z },  // hinten, oben, rechts
				{ min_x, max_y, min_z },  // hinten, oben, links
		};
		obj.bbox = bbox;
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


