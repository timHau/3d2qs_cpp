//
// Created by tau on 29.01.21.
//

#ifndef INC_3D2QS_SUNC_CPP_DEBUGEXPORTER_H
#define INC_3D2QS_SUNC_CPP_DEBUGEXPORTER_H

#include <iostream>
#include <string>
#include "cpptoml.h"
#include "happly.h"
#include <filesystem>

namespace fs = std::filesystem;

class DebugExporter
{
private:
	static void handle_toml(const fs::path& toml_path, const std::string& room_id, const fs::path& input_dir);

public:
	static void to_ply(const fs::path& input_dir);
};


#endif //INC_3D2QS_SUNC_CPP_DEBUGEXPORTER_H
