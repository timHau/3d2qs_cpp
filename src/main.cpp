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
	/*
	SUNCTransformer::transform("../data/sunc/");
	MatterportTransformer::transform("../data/matterport3d/region_segmentations/");

	for (auto & f : fs::directory_iterator("../data")) {
		std::cout << f.path() << std::endl;
	}
	 */

	auto config = cpptoml::parse_file("../data/matterport3d/config/matterport3d.toml");
	auto val = config->get_qualified_as<std::string>("dataset.name");
	std::vector<Object> objects_matterport;
	for (const auto& obj : *config->get_table_array("object"))
	{
		Object object{ obj };
		objects_matterport.emplace_back(object);
	}

	/*
	fs::path xml_output_path_matterport("../data/matterport3d/xml/matterport3d.xml");
	XmlExporter::to_xml(xml_output_path_matterport, objects_matterport);

	DebugExporter::to_ply("../data/sunc/config/");
	DebugExporter::to_ply("../data/matterport3d/config/");
	 */

	auto obj_a = objects_matterport[25];
	auto obj_b = objects_matterport[70];
	auto rel_ab = obj_a.relation_to(obj_b);
	auto rel_ba = obj_b.relation_to(obj_a);
	std::cout << obj_a.get_id()->c_str() << " " << rel_ab << " " << obj_b.get_id()->c_str() << std::endl;
	std::cout << obj_b.get_id()->c_str() << " " << rel_ba << " " << obj_a.get_id()->c_str() << std::endl;

	return 0;
}
