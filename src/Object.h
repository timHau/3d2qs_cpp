#ifndef INC_3D2QS_SUNC_CPP_OBJECT_H
#define INC_3D2QS_SUNC_CPP_OBJECT_H

#include "cpptoml.h"
#include <Eigen/Dense>
#include <iostream>
#include <optional>
#include <tinyxml2.h>

#define _USE_MATH_DEFINES

#include <cmath>

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
	Eigen::Matrix<double, 4, 4, Eigen::ColMajor> _transform; // column major
	Eigen::Matrix4d _cam_transform;

	bool bbox_vertices_equal_to(Object obj_b) const;

	bool is_tangent_to(Object& obj_b);

	static std::optional<Eigen::Vector3d> get_intersection_of_line_with_bbox(
			double dist_1,
			double dist_2,
			const Eigen::Vector3d& p1,
			const Eigen::Vector3d& p2
	);

	bool is_inside_box(const Eigen::Vector3d& p);

	std::pair<Eigen::Vector3d, Eigen::Vector3d> get_min_max_bbox();

	double get_distance_to(Object& obj_b);

	double get_volume() const;

	bool is_close_enough(Object& obj_b);

	std::tuple<double, double, double> get_projected_diff(Object& obj_b, Eigen::Matrix4d& transform);

	std::string relative_side_of(Object& obj_b);

	std::string intrinsic_side_of(Object& obj_b);

public:
	explicit Object(const std::shared_ptr<cpptoml::table>& obj);

	std::vector<Eigen::Vector3d>* get_bbox_vertices();

	std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>* get_bbox_edges();

	std::vector<std::vector<Eigen::Vector3d>>* get_bbox_faces();

	std::vector<Eigen::Vector3d>* get_bbox_normals();

	std::string* get_label();

	std::string* get_id();

	Eigen::Vector3d* get_centroid();

	std::string relation_to(Object& obj_b);

	std::optional<std::string> intrinsic_orientation_to(Object& obj_b);

	std::optional<std::string> relative_orientation_to(Object& obj_b);

	tinyxml2::XMLElement* as_xml(tinyxml2::XMLDocument& doc);

	int count_inside_bb(std::vector<Eigen::Vector3d>& obj_b_bbox);

	int count_lines_inside_bb(std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>& obj_b_lines_bbox);

};

#endif //INC_3D2QS_SUNC_CPP_OBJECT_H
