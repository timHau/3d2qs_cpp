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


public:
	static void transform(const std::string& path);
};


#endif //INC_3D2QS_SUNC_CPP_MATTERPORTTRANSFORMER_H
