#ifndef INC_3D2QS_SUNC_CPP_OBJECT_H
#define INC_3D2QS_SUNC_CPP_OBJECT_H

#include "cpptoml.h"
#include <Eigen/Dense>
#include <optional>
#include <tinyxml2.h>

struct BoundingBox
{
	std::vector<Eigen::Vector3d> vertices;
	std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> edges;
	std::vector<std::vector<Eigen::Vector3d>> faces;
	std::vector<Eigen::Vector3d> normals;
};

class Object
{
private:
	std::string _label;
	std::string _id;
	Eigen::Vector3d _centroid;
	double _volume;
	BoundingBox _bbox;

	void init_bbox(const std::shared_ptr<cpptoml::table>& obj);

	bool is_equal_to(Object obj_b);

	bool is_tangent_to(Object& obj_b);

	int count_inside_bb(std::vector<Eigen::Vector3d>& obj_b_bbox);

	int count_lines_inside_bb(std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>& obj_b_lines_bbox);

	std::optional<Eigen::Vector3d>
	get_intersection(double dist_1, double dist_2, const Eigen::Vector3d& p1, const Eigen::Vector3d& p2);

	bool is_inside_box(const Eigen::Vector3d& p);

	std::pair<Eigen::Vector3d, Eigen::Vector3d> get_min_max_bbox();

	double get_distance_to(Object obj_b);

	double get_volume() const;

	Eigen::Vector3d get_centroid();

	void plane_intersection(Object& obj_b);

public:
	explicit Object(const std::shared_ptr<cpptoml::table>& obj);

	std::vector<Eigen::Vector3d>* get_bbox();

	std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>* get_bbox_lines();

	std::string* get_label();

	std::string* get_id();

	std::string relation_to(Object& obj_b);

	int side_of(const Object& obj_b);

	std::optional<std::string> intrinsic_orientation_to(const Object& obj_b);

	tinyxml2::XMLElement* as_xml(tinyxml2::XMLDocument& doc);
};

#endif //INC_3D2QS_SUNC_CPP_OBJECT_H
