#ifndef INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
#define INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H

#include "happly.h"
#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include "cpptoml.h"
#include <Eigen/Dense>
#include <filesystem>

namespace fs = std::filesystem;

struct Face
{
	int seg_ind; // index of the segment that this face is contained in
	std::vector<std::vector<double>> vertices; // 3 vertices that build the face
};

struct Obj
{
	std::string category_index;
	std::string object_index;
	std::vector<double> obb;
	std::string catergory_name;
	std::vector<int> segments_indices;
	std::vector<double> centroid;
	std::vector<double> axes_length;
	std::vector<double> dominant_normal;
	std::vector<double> normalized_axes;
	std::vector<Face> faces;
	std::vector<std::vector<double>> bbox;
};

class MatterportTransformer
{
private:
	static void handle_house(
			std::map<std::string, std::vector<std::string>>& house,
			const fs::path& matterport_path,
			const fs::path& house_path,
			const std::string& house_name
	);

	static std::map<std::string, std::vector<std::string>> read_house_file(
			const fs::path& house_path,
			const std::string& house_name
	);

	static void write_objects_to_toml(
			std::vector<Obj>& objects,
			const fs::path& config_dir,
			const std::string& region_name
	);

	static std::vector<Obj> get_objects_per_region(
			std::map<std::string, std::vector<std::string>>& house,
			const fs::path& house_path,
			const std::string& house_name,
			const fs::path& matterport_path,
			std::string& region_id
	);

	static std::vector<std::string> split_line(const std::string& line);

	static std::vector<std::string> get_column_tsv(
			const std::string& file_name,
			int column
	);

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
