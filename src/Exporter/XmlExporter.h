#ifndef INC_3D2QS_SUNC_CPP_XMLEXPORTER_H
#define INC_3D2QS_SUNC_CPP_XMLEXPORTER_H

#include <iostream>
#include <tinyxml2.h>
#include <filesystem>
#include "../Object.h"

namespace fs = std::filesystem;

class XmlExporter
{
public:
	static void to_xml(const fs::path& output_path, std::vector<Object>& objects);
};


#endif //INC_3D2QS_SUNC_CPP_XMLEXPORTER_H
