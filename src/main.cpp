#include <iostream>
#include "cpptoml.h"
#include "Object.h"
#include <filesystem>

#include "Transformer/SUNCTransformer.h"
#include "Transformer/MatterportTransformer.h"

#include "Exporter/DebugExporter.h"
#include "Exporter/XmlExporter.h"

namespace fs = std::filesystem;

struct Dataset
{
	const fs::path xml_path;
	const fs::path config_path;
	std::vector<Object> objects;
};

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

	std::vector<Dataset> datasets;
	for (auto& dir : fs::directory_iterator("../data/datasets"))
	{
		if (!fs::is_directory(dir))
			continue;
		const fs::path& dataset_path = dir.path();
		const fs::path& config_path = dataset_path / "config";

		for (auto& config_file : fs::directory_iterator(config_path))
		{
			if (!(config_file.path().extension() == ".toml"))
				continue; // ignore all files that are not .toml files

			auto config = cpptoml::parse_file(config_file.path());
			std::vector<Object> objects;
			for (const auto& obj : *config->get_table_array("object"))
			{
				Object object{ obj };
				objects.emplace_back(object);
			}

			std::string xml_filename = config_file.path().stem().string() + ".xml";
			const fs::path& xml_path = dataset_path / "xml" / xml_filename;
			Dataset d{ xml_path, config_path, objects };
			datasets.push_back(d);
		}
	}


	/*
	// for debugging only
	// ---------------------------------------------------------------------
	auto obj_a = datasets[1].objects[29];
	auto obj_b = datasets[1].objects[15];
	auto rel_ba = obj_a.relative_orientation_to(obj_b);
	if (rel_ba)
	{
		std::cout << obj_b.get_id()->c_str() << " " << rel_ba.value() << " " << obj_a.get_id()->c_str()
				  << std::endl;
		std::cout << "-------" << std::endl;
	}
	else
	{
		std::cout << "either not the smaller one or not close enough" << std::endl;
	}
	std::cout << "obj a:" << *(obj_a.get_id()) << "_" << *(obj_a.get_label()) << " obj_b " << *(obj_b.get_id()) << "_"
			  << *(obj_b.get_label()) << std::endl;
	// ---------------------------------------------------------------------
	*/

	bool should_stop = false;
	while (!should_stop)
	{
		std::string input = ask();

		if (input == "r")
		{
			SUNCTransformer::transform("../data/datasets/sunc/", true);
			MatterportTransformer::transform("../data/datasets/matterport3d/", "1pXnuDYAj8r", true);
		}

		if (input == "x")
		{
			for (auto& dataset : datasets)
			{
				XmlExporter::to_xml(dataset.xml_path, dataset.objects);
			}
		}

		if (input == "d")
		{
			for (auto& dataset : datasets)
			{
				DebugExporter::to_ply(dataset.config_path);
			}
		}

		if (input == "q")
			should_stop = true;
	}

	return 0;
}
