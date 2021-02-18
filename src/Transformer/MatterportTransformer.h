#ifndef INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
#define INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H

#include "happly.h"
#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include "cpptoml.h"
#include <filesystem>

namespace fs = std::filesystem;

class MatterportTransformer
{
private:
	static void handle_region(
			const fs::path& region_path,
			const std::string& region,
			const fs::path& output_path);

	static void handle_house(
			const fs::path& house_path,
			const std::string& house_name,
			const std::shared_ptr<cpptoml::table>& root,
			const std::vector<std::string>& categories);

	static void handle_object(
			const nlohmann::json& seg_group,
			const std::shared_ptr<cpptoml::table_array>& object_table_array,
			const std::vector<std::string>& categories);

	static std::vector<std::string> get_column_tsv(
			const std::string& file_name,
			int column);

public:
	static void transform(const std::string& path);
};


#endif //INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
