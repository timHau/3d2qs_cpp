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
	SUNCTransformer::transform("../data/datasets/sunc/");
	MatterportTransformer::transform("../data/datasets/matterport3d/");

	/*
	for (auto & f : fs::directory_iterator("../data/datasets")) {
		std::cout << f.path() << std::endl;
	}
	 */

	auto config = cpptoml::parse_file("../data/datasets/sunc/config/0004d52d1aeeb8ae6de39d6bd993e992.toml");
	auto val = config->get_qualified_as<std::string>("dataset.name");
	std::vector<Object> objects_matterport;
	for (const auto& obj : *config->get_table_array("object"))
	{
		Object object{ obj };
		objects_matterport.emplace_back(object);
	}

	/*
	fs::path xml_output_path_matterport("../data/datasets/matterport3d/xml/matterport3d.xml");
	XmlExporter::to_xml(xml_output_path_matterport, objects_matterport);
	 */

	DebugExporter::to_ply("../data/datasets/sunc/config/");
	/*
	DebugExporter::to_ply("../data/datasets/matterport3d/config/");
	 */

	/*
	auto obj_a = objects_matterport[10];
	auto obj_b = objects_matterport[13];
	auto rel_ab = obj_a.intrinsic_orientation_to(obj_b);
	if (rel_ab)
	{
		std::cout << obj_a.get_id()->c_str() << " " << rel_ab.value() << " " << obj_b.get_id()->c_str()
				  << std::endl;
		std::cout << "-------" << std::endl;
	}
	 */

	return 0;
}
