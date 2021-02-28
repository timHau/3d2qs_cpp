#ifndef INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H
#define INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H

#include <iostream>
#include <fstream>
#include "json.hpp"
#include "cpptoml.h"
#include <Eigen/Dense>
#include <filesystem>
#include "happly.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace fs = std::filesystem;

struct SuncObject {
	std::string id;
	std::string room_id;
	std::vector<Eigen::Vector3d> vertices;
};

class SUNCTransformer
{
private:
	static void handle_room(
			const nlohmann::json& json_data,
			const std::string& room_id,
			const fs::path& object_path,
			const fs::path& objects_path,
			const fs::path& output_path,
			bool debug);

	static SuncObject handle_object(fs::path& obj_path, const std::string& room_id, Eigen::Matrix4d& transform, Eigen::Matrix3d& rot);

	static void write_object_to_ply(SuncObject& object, const fs::path& objects_path);

public:
	static void transform(const std::string& path);

	static void transform(const std::string& path, bool debug);
};


#endif //INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H
