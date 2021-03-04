#ifndef INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
#define INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H

#include "happly.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "json.hpp"
#include "cpptoml.h"
#include <Eigen/Dense>
#include <filesystem>

namespace fs = std::filesystem;

struct Face
{
	int seg_ind; // index of the segment that this face is contained in
	std::vector<double> vertices; // 3 vertices that build the face [x1, y1, z1, x2, y2, z2, x3, y3, z3]
};

struct Segment
{
	int seg_ind; // index of segment
	std::vector<Face> faces;
};

struct MatterportObject
{
	std::string id;
	std::string label;
	std::vector<double> centroid;
	std::vector<double> axes_length;
	std::vector<double> dominant_normal;
	std::vector<double> normalized_axes;
	std::vector<Segment> segments;
	std::vector<std::string> cam_transform;
	std::vector<std::vector<double>> bbox;
};

class MatterportTransformer
{
private:
	static void
	handle_house(std::shared_ptr<cpptoml::table>& root,
			std::map<std::string, std::vector<std::string>>& obj_camera_mapper,
			const fs::path& matterport_path,
			const fs::path& house_path,
			const std::string& house_name,
			bool debug);

	static std::shared_ptr<cpptoml::table>
	object_to_toml(MatterportObject& obj, std::vector<std::string>& all_categories);

	static std::map<int, Segment> get_all_segments(const fs::path& house_path, const std::string& house_name);

	static std::vector<std::vector<double>> get_bbox(MatterportObject& obj);

	static std::vector<std::string> get_all_categories(const fs::path& matterport_path);

	static void write_object_to_ply(MatterportObject& obj, const fs::path& config_obj_dir);

	static std::map<std::string, std::vector<std::string>> get_obj_camera_mapper(std::string& house_buffer);

public:
	static void transform(const std::string& path, const std::string& house_name);

	static void transform(const std::string& path, const std::string& house_name, bool debug);

};


#endif //INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
