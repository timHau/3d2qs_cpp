#ifndef INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H
#define INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H

#include <iostream>
#include <fstream>
#include "json.hpp"
#include "cpptoml.h"
#include <Eigen/Dense>
#include "tiny_obj_loader.h"
#include <filesystem>

#define _USE_MATH_DEFINES // for C++
#include <cmath>

namespace fs = std::filesystem;

class SUNCTransformer
{
private:
	static void handle_room(const nlohmann::json& json_data, const fs::path& output_path);

public:
	static void transform(const std::string& path);
};


#endif //INC_3D2QS_SUNC_CPP_SUNCTRANSFORMER_H
