#include "MatterportTransformer.h"

void MatterportTransformer::transform(const std::string& path)
{
	transform(path, false);
}

void MatterportTransformer::transform(const std::string& path, bool debug)
{
	std::shared_ptr<cpptoml::table> root = cpptoml::make_table();

	const fs::path matterport_path(path);
	if (!fs::exists(matterport_path))
		throw std::invalid_argument("matterport directory does not exist");
	const std::string house_name = "1pXnuDYAj8r";
	std::cout << "start parsing " << house_name << " (matterport)" << std::endl;

	handle_house(root, matterport_path, house_name, debug);

	auto meta_table = cpptoml::make_table();
	meta_table->insert("name", house_name);
	root->insert("dataset", meta_table);
	const fs::path config_path = matterport_path / "config";
	if (!fs::exists(config_path))
		throw std::invalid_argument("config directory does not exist");
	const fs::path output_path = config_path / (house_name + ".toml");
	std::ofstream output;
	output.open(output_path);
	output << (*root);
	output.close();
	std::cout << "wrote: " << output_path << std::endl;
}

void MatterportTransformer::handle_house(
		std::shared_ptr<cpptoml::table>& root,
		const fs::path& matterport_path,
		const std::string& house_name,
		bool debug)
{
	const fs::path house_path = matterport_path / "house_segmentations";
	if (!fs::exists(house_path))
		throw std::invalid_argument("house segmentations directory does not exist");

	// get all segments in the house
	std::cout << "start building segments " << std::endl;
	std::map<int, Segment> all_segs = get_all_segments(house_path, house_name);
	std::cout << "finished building: " << all_segs.size() << " segments" << std::endl;

	const fs::path semseg_path = house_path / (house_name + ".semseg.json");
	if (!fs::exists(semseg_path))
		throw std::invalid_argument("house .semseg.json does not exist");
	std::ifstream semseg_house_stream(semseg_path);
	nlohmann::json semseg_house_json;
	semseg_house_stream >> semseg_house_json;
	auto seg_groups = semseg_house_json["segGroups"];

	const fs::path debug_dir = matterport_path / "debug";
	if (!fs::exists(debug_dir))
		fs::create_directory(debug_dir);

	const fs::path config_obj_dir = debug_dir / house_name;
	if (!fs::exists(config_obj_dir))
		fs::create_directory(config_obj_dir);

	// get all categories
	auto all_categories = get_all_categories(matterport_path);

	std::cout << "start collecting objects" << std::endl;
	auto object_table_array = cpptoml::make_table_array();
	for (const auto& seg_group : seg_groups)
	{
		// each seg_group represents one object
		int id = seg_group["id"];
		int label_index = seg_group["label_index"];
		if (label_index == 40 || label_index < 0) // skip over unknown objects
			continue;
		std::string label = all_categories[label_index];
		std::vector<double> centroid = seg_group["obb"]["centroid"];
		std::vector<double> axes_lengths = seg_group["obb"]["axesLengths"];
		std::vector<double> dominant_normal = seg_group["obb"]["dominantNormal"];
		std::vector<double> normalized_axes = seg_group["obb"]["normalizedAxes"];
		std::vector<int> segment_indices = seg_group["segments"];
		std::vector<Segment> segments;
		for (int seg_index : segment_indices)
			segments.push_back(all_segs[seg_index]);

		MatterportObject obj{ id, label, centroid, axes_lengths, dominant_normal, normalized_axes, segments };
		obj.bbox = get_bbox(obj);

		auto as_toml = object_to_toml(obj, all_categories);
		object_table_array->push_back(as_toml);

		root->insert("object", object_table_array);

		if (debug)
			write_object_to_ply(obj, config_obj_dir);
	}
	std::cout << "finished collecting objects" << std::endl;
}

std::map<int, Segment>
MatterportTransformer::get_all_segments(const fs::path& house_path, const std::string& house_name)
{
	// get face information
	const fs::path fseg_path = house_path / (house_name + ".fsegs.json");
	if (!fs::exists(fseg_path))
		throw std::invalid_argument("house .fsegs.json does not exists");
	std::ifstream fseg_stream(fseg_path);
	nlohmann::json fseg_json;
	fseg_stream >> fseg_json;
	std::vector<int> fseg_indices = fseg_json["segIndices"];

	std::cout << "start loading house: " << house_name << ".ply" << std::endl;
	const fs::path house_ply_path = house_path / (house_name + ".ply");
	if (!fs::exists(house_ply_path))
		throw std::invalid_argument("house .ply does not exists");
	happly::PLYData ply_file(house_ply_path);
	std::vector<double> vert_x = ply_file.getElement("vertex").getProperty<double>("x");
	std::vector<double> vert_y = ply_file.getElement("vertex").getProperty<double>("y");
	std::vector<double> vert_z = ply_file.getElement("vertex").getProperty<double>("z");
	std::vector<std::vector<int>> vertex_indices = ply_file.getElement("face").getListProperty<int>("vertex_indices");
	std::cout << "finished loading house: " << house_name << ".ply" << std::endl;

	std::cout << "collecting all segments" << std::endl;
	// every entry in the array 'segIndices' corresponds to one face
	// the i-th face is contained in the segment with id segIndices[i]
	std::map<int, Segment> all_segments;
	int i = 0;
	for (int seg_ind : fseg_indices)
	{
		std::vector<int> vert_indices = vertex_indices[i];
		std::vector<double> vertices = {
				vert_x[vert_indices[0]], vert_y[vert_indices[0]], vert_z[vert_indices[0]],
				vert_x[vert_indices[1]], vert_y[vert_indices[1]], vert_z[vert_indices[1]],
				vert_x[vert_indices[2]], vert_y[vert_indices[2]], vert_z[vert_indices[2]],
		};
		Face f{ seg_ind, vertices };
		if (all_segments.find(seg_ind) != all_segments.end())
			// segment already exists ==> add to faces
			all_segments[seg_ind].faces.push_back(f);
		else
		{
			// segment does not exists ==> create segment, add to map
			std::vector<Face> faces{ f };
			Segment s{ seg_ind, faces };
			all_segments[seg_ind] = s;
		}
		i++;
	}

	return all_segments;
}

std::vector<std::vector<double>> MatterportTransformer::get_bbox(MatterportObject& obj)
{
	// there should be a better way to do this, i know it is a brute force way

	// calculate the bounding box
	double min_x = std::numeric_limits<double>::infinity(), max_x = -std::numeric_limits<double>::infinity();
	double min_y = std::numeric_limits<double>::infinity(), max_y = -std::numeric_limits<double>::infinity();
	double min_z = std::numeric_limits<double>::infinity(), max_z = -std::numeric_limits<double>::infinity();

	for (const Segment& s : obj.segments)
		for (const Face& f: s.faces)
			for (int i = 0; i < 9; i += 3)
			{
				if (f.vertices[i + 0] < min_x) min_x = f.vertices[i + 0];
				if (f.vertices[i + 0] > max_x) max_x = f.vertices[i + 0];
				if (f.vertices[i + 1] < min_y) min_y = f.vertices[i + 1];
				if (f.vertices[i + 1] > max_y) max_y = f.vertices[i + 1];
				if (f.vertices[i + 2] < min_z) min_z = f.vertices[i + 2];
				if (f.vertices[i + 2] > max_z) max_z = f.vertices[i + 2];
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

	return bbox;
}

std::shared_ptr<cpptoml::table>
MatterportTransformer::object_to_toml(MatterportObject& obj, std::vector<std::string>& all_categories)
{
	auto object_table = cpptoml::make_table();
	// create transform matrix from obb
	// ref: https://stackoverflow.com/questions/53227533/how-to-find-out-the-rotation-matrix-for-the-oriented-bounding-box
	Eigen::Matrix<double, 4, 4, Eigen::ColMajor> t;
	t << obj.normalized_axes[0], obj.normalized_axes[3], obj.normalized_axes[6], obj.centroid[0],
		 obj.normalized_axes[1], obj.normalized_axes[4], obj.normalized_axes[7], obj.centroid[1],
		 obj.normalized_axes[2], obj.normalized_axes[5], obj.normalized_axes[8], obj.centroid[2],
		 0.0, 0.0, 0.0, 1.0;
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
	object_table->insert("id", std::to_string(obj.id));
	object_table->insert("label", obj.label);
	object_table->insert("transform", transform_array);
	return object_table;
}

std::vector<std::string> MatterportTransformer::get_all_categories(const fs::path& matterport_path)
{
	const fs::path metadata_dir = matterport_path / "metadata";
	if (!fs::exists(metadata_dir))
		throw std::invalid_argument("metadata directory does not exist");
	std::string file_name = metadata_dir / "category_mapping.tsv";
	if (!fs::exists(file_name))
		throw std::invalid_argument("category mapping does not exist");
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

		if (2 < tokens.size())
			res.push_back(token);
	}

	return res;
}

void MatterportTransformer::write_object_to_ply(MatterportObject& obj, const fs::path& config_obj_dir)
{
	std::vector<double> vert_x_out;
	std::vector<double> vert_y_out;
	std::vector<double> vert_z_out;
	std::vector<std::vector<int>> vert_indices_out;

	int index_count = 0;
	for (auto& segment : obj.segments)
	{
		for (auto& face : segment.faces)
		{
			for (int i = 0; i < 9; i += 3)
			{
				vert_x_out.push_back(face.vertices[i + 0]);
				vert_y_out.push_back(face.vertices[i + 1]);
				vert_z_out.push_back(face.vertices[i + 2]);
			}

			std::vector<int> vert_index{ index_count, index_count + 1, index_count + 2 };
			vert_indices_out.push_back(vert_index);
			index_count += 3;
		}
	}

	const fs::path obj_path = config_obj_dir / (std::to_string(obj.id) + "_" + obj.label + ".ply");

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