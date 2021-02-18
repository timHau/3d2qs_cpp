#include "MatterportTransformer.h"

struct Face
{
	int seg_ind; // index of the segment that this face is contained in
	std::vector<std::vector<double>> vertices; // 3 vertices that build the face
};


void MatterportTransformer::transform(const std::string& path)
{
	std::cout << "start parsing matterport3d data" << std::endl;

	const fs::path matterport_path(path);
	const fs::path config_path = matterport_path / "config";
	const fs::path region_path = matterport_path / "region_segmentations";
	const fs::path house_path = matterport_path / "house_segmentations";
	const fs::path metadata_path = matterport_path / "metadata";

	auto categories = get_column_tsv(metadata_path / "category_mapping.tsv", 2);

	// TODO loop over houses
	std::shared_ptr<cpptoml::table> root = cpptoml::make_table();

	const std::string house_name = "1pXnuDYAj8r";
	handle_house(house_path, house_name, root, categories);

	auto meta_table = cpptoml::make_table();
	meta_table->insert("name", house_name);
	root->insert("dataset", meta_table);

	const fs::path output_path = config_path / (house_name + ".toml");
	std::ofstream output;
	output.open(output_path);
	output << (*root);
	output.close();
	std::cout << "wrote: " << output_path << std::endl;
}

void MatterportTransformer::handle_house(
		const fs::path& house_path,
		const std::string& house_name,
		const std::shared_ptr<cpptoml::table>& root,
		const std::vector<std::string>& categories
)
{
	std::ifstream semseg_stream(house_path / (house_name + ".semseg.json"));
	nlohmann::json semseg_json;
	semseg_stream >> semseg_json;

	auto object_table_array = cpptoml::make_table_array();
	for (const auto& seg_group : semseg_json["segGroups"])
	{
		handle_object(seg_group, object_table_array, categories);
	}
	root->insert("object", object_table_array);
}

void MatterportTransformer::handle_object(
		const nlohmann::json& seg_group,
		const std::shared_ptr<cpptoml::table_array>& object_table_array,
		const std::vector<std::string>& categories
)
{
	int label_index = seg_group["label_index"];
	auto id = std::to_string((int) seg_group["id"]);
	auto obb = seg_group["obb"];
	auto segments = seg_group["segments"];
	std::cout << categories[label_index] << std::endl;
	std::cout << obb << std::endl;

	auto centroid_array = cpptoml::make_array();
	std::vector<double> centroid = obb["centroid"];
	for (auto& c_i : centroid)
		centroid_array->push_back(c_i);

	auto object_table = cpptoml::make_table();
	// object_table->insert("bbox", bbox_array);
	object_table->insert("id", id);
	object_table->insert("label", categories[label_index]);
	object_table->insert("centroid", centroid_array);
	object_table_array->push_back(object_table);
}

std::vector<std::string> MatterportTransformer::get_column_tsv(const std::string& file_name, int column)
{
	std::vector<std::string> res = {};

	std::ifstream file(file_name);
	std::string line;
	while (std::getline(file, line))
	{
		std::vector<std::string> parts;
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
