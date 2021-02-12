#include "XmlExporter.h"
#include "../Utils.h"

void XmlExporter::to_xml(const fs::path &output_path, std::vector<Object> &objects) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *root = doc.NewElement("ROOT");

    // write spatial entities into .xml file
    for (Object &obj : objects) {
        root->InsertEndChild(obj.as_xml(doc));
    }

    // write qslinks into .xml file
    for (auto &obj_pair : utils::cartesian_product(objects, objects)) {
        Object obj_a = obj_pair.first;
        Object obj_b = obj_pair.second;
        std::string rel = obj_a.relation_to(obj_b);
        if (rel != "DC") {
            tinyxml2::XMLElement *qslink = doc.NewElement("QSLINK");
            qslink->SetAttribute("relType", rel.c_str());
            qslink->SetAttribute("figure", obj_a.get_label()->c_str());
            qslink->SetAttribute("ground", obj_b.get_label()->c_str());
            root->InsertEndChild(qslink);
        }
    }

    doc.InsertFirstChild(root);
    doc.SaveFile(output_path.c_str());
    std::cout << "wrote xml: " << output_path.c_str() << std::endl;
}
