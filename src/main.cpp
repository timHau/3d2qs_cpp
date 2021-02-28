#include <iostream>
#include "cpptoml.h"
#include "Object.h"
#include <filesystem>

#include "Transformer/SUNCTransformer.h"
#include "Transformer/MatterportTransformer.h"

#include "Exporter/DebugExporter.h"
#include "Exporter/XmlExporter.h"

namespace fs = std::filesystem;

int main()
{
	SUNCTransformer::transform("../data/datasets/sunc/", true);
	// MatterportTransformer::transform("../data/datasets/matterport3d/");

	/*
	for (auto& dir : fs::directory_iterator("../data/datasets")) {
		const fs::path& dataset_path = dir.path();
		const fs::path& config_path = dataset_path / "config";

		for (auto& config_file : fs::directory_iterator(config_path))
		{
			 if(!(config_file.path().extension() == ".toml"))
				 continue; // ignore all files that are not .toml

			auto config = cpptoml::parse_file(config_file.path());
			std::vector<Object> objects;
			for (const auto& obj : *config->get_table_array("object"))
			{
				Object object{ obj };
				objects.emplace_back(object);
			}
		}
	}
	*/

	/*
	fs::path xml_output_path_matterport("../data/datasets/matterport3d/xml/matterport3d.xml");
	XmlExporter::to_xml(xml_output_path_matterport, objects_matterport);
	 */

	/*
	DebugExporter::to_ply("../data/datasets/sunc/config/");
	DebugExporter::to_ply("../data/datasets/matterport3d/config/");
	 */

	auto config = cpptoml::parse_file("../data/datasets/matterport3d/config/1pXnuDYAj8r.toml");
	auto val = config->get_qualified_as<std::string>("dataset.name");
	std::vector<Object> objects_matterport;
	for (const auto& obj : *config->get_table_array("object"))
	{
		Object object{ obj };
		objects_matterport.emplace_back(object);
	}

	auto obj_a = objects_matterport[48];
	auto obj_b = objects_matterport[0];
	auto rel_ab = obj_a.intrinsic_orientation_to(obj_b);
	if (rel_ab)
	{
		std::cout << obj_a.get_id()->c_str() << " " << rel_ab.value() << " " << obj_b.get_id()->c_str()
				  << std::endl;
		std::cout << "-------" << std::endl;
	}
	std::cout << "obj a:" << *(obj_a.get_id()) << "_" << *(obj_a.get_label()) << " obj_b " << *(obj_b.get_id()) << "_" << *(obj_b.get_label()) << std::endl;

	return 0;
}
