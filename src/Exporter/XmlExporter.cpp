#include "XmlExporter.h"

void XmlExporter::to_xml(const fs::path &output_path, std::vector<Object> &objects) {
    tinyxml2::XMLDocument doc;

    for (Object &obj : objects) {
        doc.InsertEndChild(obj.as_xml(doc));
    }

    doc.SaveFile(output_path.c_str());
}
