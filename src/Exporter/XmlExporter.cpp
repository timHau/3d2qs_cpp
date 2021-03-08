#include "XmlExporter.h"

void XmlExporter::to_xml(const fs::path& output_path, std::vector<Object>& objects)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement* root = doc.NewElement("ROOT");

	// write spatial entities into .xml file
	for (Object& obj : objects)
	{
		root->InsertEndChild(obj.as_xml(doc));
	}

	// write qslinks / olinks into .xml file
	for (auto& obj_pair : utils::cartesian_product(objects, objects))
	{
		Object obj_a = obj_pair.first;
		Object obj_b = obj_pair.second;
		// for qslink
		std::string rel = obj_a.relation_to(obj_b);
		if (rel != "DC" && rel != "EQ")
		{
			tinyxml2::XMLElement* qslink = doc.NewElement("QSLINK");
			qslink->SetAttribute("relType", rel.c_str());
			qslink->SetAttribute("figure", obj_a.get_label()->c_str());
			qslink->SetAttribute("fromId", obj_a.get_id()->c_str());
			qslink->SetAttribute("ground", obj_b.get_label()->c_str());
			qslink->SetAttribute("toId", obj_b.get_id()->c_str());
			root->InsertEndChild(qslink);
		}

		// for relative olink
		auto relative_rel = obj_a.relative_orientation_to(obj_b);
		if (relative_rel)
		{
			tinyxml2::XMLElement* rel_olink = doc.NewElement("OLINK");
			rel_olink->SetAttribute("frame_type", "relative");
			rel_olink->SetAttribute("fromId", obj_a.get_id()->c_str());
			rel_olink->SetAttribute("relType", relative_rel->c_str());
			rel_olink->SetAttribute("toId", obj_b.get_id()->c_str());
			root->InsertEndChild(rel_olink);
		}

		// for intrinsic olink
		auto intrinsic_rel = obj_a.intrinsic_orientation_to(obj_b);
		if (intrinsic_rel)
		{
			tinyxml2::XMLElement* intr_olink = doc.NewElement("OLINK");
			intr_olink->SetAttribute("frame_type", "intrinsic");
			intr_olink->SetAttribute("fromId", obj_a.get_id()->c_str());
			intr_olink->SetAttribute("relType", relative_rel->c_str());
			intr_olink->SetAttribute("toId", obj_b.get_id()->c_str());
			root->InsertEndChild(intr_olink);
		}

	}

	doc.InsertFirstChild(root);
	doc.SaveFile(output_path.c_str());
	std::cout << "wrote xml: " << output_path.c_str() << std::endl;
}
