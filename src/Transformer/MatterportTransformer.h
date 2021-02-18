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
	static void handle_house(
			const fs::path& matterport_path,
			const std::string& house_name,
			const std::shared_ptr<cpptoml::table>& root
	);

	static std::vector<std::string> get_column_tsv(
			const std::string& file_name,
			int column
	);

	static std::map<std::string, std::vector<std::string>> read_house_file(
			const fs::path& house_path,
			const std::string& house_name
	);

	static std::vector<std::string> split_line(const std::string& line);

	static void write_as_ply(
			const fs::path& out_path,
			std::vector<double>& vert_x_out,
			std::vector<double>& vert_y_out,
			std::vector<double>& vert_z_out,
			std::vector<std::vector<int>>& vert_indices_out
	);

public:
	static void transform(const std::string& path);
};


#endif //INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
