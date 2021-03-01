#include <iostream>
#include "cpptoml.h"
#include "Object.h"
#include <filesystem>

#include "Transformer/SUNCTransformer.h"
#include "Transformer/MatterportTransformer.h"

#include "Exporter/DebugExporter.h"
#include "Exporter/XmlExporter.h"

namespace fs = std::filesystem;

std::string ask()
{
	std::cout << "................................." << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "[r] Read Datasets to toml" << std::endl;
	std::cout << "[x] Export to xml" << std::endl;
	std::cout << "[d] Debug export bounding boxes" << std::endl;
	std::cout << "[q] Quit" << std::endl;
	std::cout << ">> " << std::endl;

	std::string input;
	std::cin >> input;
	return input;
}

int main()
{
	std::cout << ".............Welcome............." << std::endl;

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

	bool should_stop = false;
	while (!should_stop)
	{
		std::string input = ask();

		if (input == "r")
		{
			SUNCTransformer::transform("../data/datasets/sunc/", true);
			MatterportTransformer::transform("../data/datasets/matterport3d/", true);
		}

		if (input == "x")
		{
			/*
			fs::path xml_output_path_matterport("../data/datasets/matterport3d/xml/matterport3d.xml");
			XmlExporter::to_xml(xml_output_path_matterport, objects_matterport);
			 */
		}

		if (input == "d")
		{
			DebugExporter::to_ply("../data/datasets/sunc/config/");
			DebugExporter::to_ply("../data/datasets/matterport3d/config/");
		}

		if (input == "q")
		{
			should_stop = true;
		}
	}


	auto config = cpptoml::parse_file("../data/datasets/sunc/config/00a2a04afad84b16ff330f9038a3d126.toml");
	auto val = config->get_qualified_as<std::string>("dataset.name");
	std::vector<Object> objects_matterport;
	for (const auto& obj : *config->get_table_array("object"))
	{
		Object object{ obj };
		objects_matterport.emplace_back(object);
	}

	auto obj_a = objects_matterport[3];
	auto obj_b = objects_matterport[50];
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
