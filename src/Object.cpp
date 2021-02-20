#include "Object.h"

Object::Object(const std::shared_ptr<cpptoml::table>& obj)
{
	_label = *(obj->get_qualified_as<std::string>("label"));
	_id = *(obj->get_qualified_as<std::string>("id"));

	// intialize bounding box
	auto box = obj->get_array_of<cpptoml::array>("bbox");

	// vertices of bounding box
	std::vector<Eigen::Vector3d> vertices = {};
	for (int i = 0; i < 8; ++i)
	{
		cpptoml::option<std::vector<double>> point = (*box)[i]->get_array_of<double>();
		Eigen::Vector3d v((*point)[0], (*point)[1], (*point)[2]);
		vertices.emplace_back(v);
	}

	std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> edges;
	// edges of bounding box
	std::vector<std::pair<int, int>> pairs = {
			{ 0, 1 },
			{ 1, 2 },
			{ 2, 3 },
			{ 3, 0 },
			{ 0, 4 },
			{ 4, 5 },
			{ 5, 6 },
			{ 6, 7 },
			{ 7, 4 },
			{ 7, 3 },
			{ 6, 2 },
			{ 5, 1 },
	};
	for (std::pair<int, int> p : pairs)
	{
		std::pair<Eigen::Vector3d, Eigen::Vector3d> line(vertices[p.first], vertices[p.second]);
		edges.emplace_back(line);
	}

	const std::vector<std::vector<Eigen::Vector3d>> faces = {
			{ vertices[0], vertices[1], vertices[2], vertices[3] },  // vorne
			{ vertices[3], vertices[2], vertices[6], vertices[7] },  // oben
			{ vertices[1], vertices[5], vertices[6], vertices[2] },  // rechts
			{ vertices[4], vertices[5], vertices[1], vertices[0] },  // unten
			{ vertices[4], vertices[0], vertices[3], vertices[7] },  // links
			{ vertices[5], vertices[4], vertices[7], vertices[6] },  // hinten
	};

	std::vector<Eigen::Vector3d> normals = {
			(vertices[2] - vertices[1]).cross(vertices[0] - vertices[1]), // vorne
			(vertices[6] - vertices[2]).cross(vertices[3] - vertices[2]), // oben
			(vertices[6] - vertices[5]).cross(vertices[1] - vertices[5]), // rechts
			(vertices[0] - vertices[1]).cross(vertices[5] - vertices[1]), // unten
			(vertices[3] - vertices[0]).cross(vertices[4] - vertices[0]), // links
			(vertices[7] - vertices[4]).cross(vertices[5] - vertices[4]), // hinten
	};


	_bbox = BoundingBox{ vertices, edges, faces, normals };

	// set centroid of bounding box
	Eigen::Vector3d V = vertices[0];
	Eigen::Vector3d A = vertices[1];
	Eigen::Vector3d B = vertices[3];
	Eigen::Vector3d C = vertices[4];

	Eigen::Vector3d AV = (A - V);
	Eigen::Vector3d BV = (B - V);
	Eigen::Vector3d CV = (C - V);

	_centroid = 0.5 * (A + B + C - V);
	_volume = AV.norm() * BV.norm() * CV.norm();

	// set the transformation matrix
	std::vector<double> transform_vec = *(obj->get_array_of<double>("transform"));
	Eigen::Matrix<double, 4, 4, Eigen::ColMajor> transform_mat(transform_vec.data());
	_transform = transform_mat;
}

std::vector<Eigen::Vector3d>* Object::get_bbox_vertices()
{
	return &_bbox.vertices;
}

std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>* Object::get_bbox_edges()
{
	return &_bbox.edges;
}

std::vector<std::vector<Eigen::Vector3d>>* Object::get_bbox_faces()
{
	return &_bbox.faces;
}

std::vector<Eigen::Vector3d>* Object::get_bbox_normals()
{
	return &_bbox.normals;
}

std::pair<Eigen::Vector3d, Eigen::Vector3d> Object::get_min_max_bbox()
{
	// returns smallest and largest x,y,z point of the bounding box
	Eigen::Vector3d smallest = _bbox.vertices[0];
	Eigen::Vector3d biggest = _bbox.vertices[6];
	for (const auto& v : _bbox.vertices)
	{
		if (v.x() <= smallest.x() && v.y() <= smallest.y() && v.z() <= smallest.z()) smallest = v;
		if (v.x() >= biggest.x() && v.y() >= biggest.y() && v.z() >= biggest.z()) biggest = v;
	}
	std::pair<Eigen::Vector3d, Eigen::Vector3d> values(smallest, biggest);
	return values;
}

std::string* Object::get_label()
{
	return &_label;
}

std::string* Object::get_id()
{
	return &_id;
}

double Object::get_volume() const
{
	return _volume;
}

Eigen::Vector3d* Object::get_centroid()
{
	return &_centroid;
}

/*
 * gets the distance between this centroid and the centroid of obj_b
 */
double Object::get_distance_to(Object& obj_b)
{
	Eigen::Vector3d d = *obj_b.get_centroid() - _centroid;
	return std::abs(d.norm());
}

bool Object::bbox_vertices_equal_to(Object obj_b) const
{
	return _bbox.vertices == *obj_b.get_bbox_vertices();
}

/*
 * return the point of intersection between a line and the bounding box
 */
std::optional<Eigen::Vector3d> Object::get_intersection_of_line_with_bbox(
		double dist_1,
		double dist_2,
		const Eigen::Vector3d& p1,
		const Eigen::Vector3d& p2)
{
	if (dist_1 == dist_2)
		return std::nullopt;
	return p1 + (p2 - p1) * -(dist_1 / (dist_2 - dist_1));
}

/*
 * determine of the point p is inside this bounding box
 */
bool Object::is_inside_box(const Eigen::Vector3d& p)
{
	// get the points from bottom and top face
	std::vector<Eigen::Vector3d> bbox_a = *get_bbox_vertices();
	Eigen::Vector3d V = bbox_a[0];
	Eigen::Vector3d A = bbox_a[1];
	Eigen::Vector3d B = bbox_a[3];
	Eigen::Vector3d C = bbox_a[4];

	Eigen::Vector3d AV = (A - V);
	Eigen::Vector3d BV = (B - V);
	Eigen::Vector3d CV = (C - V);

	bool t_1 = V.dot(AV) < p.dot(AV) && p.dot(AV) < A.dot(AV);
	bool t_2 = V.dot(BV) < p.dot(BV) && p.dot(BV) < B.dot(BV);
	bool t_3 = V.dot(CV) < p.dot(CV) && p.dot(CV) < C.dot(CV);
	return (t_1 && t_2 && t_3);
}

/*
 *  test which points from obj_b bounding box is inside this bounding box, returns indices
 */
int Object::count_inside_bb(std::vector<Eigen::Vector3d>& obj_b_bbox)
{
	// number of points in obj_bs bounding box that are inside obj_as bounding box
	int count = 0;
	for (const Eigen::Vector3d& P : obj_b_bbox)
	{
		if (is_inside_box(P))
			count++;
	}
	return count;
}

/*
 * test if obj_b bounding box is inside the face of this bounding box
 */
bool Object::is_tangent_to(Object& obj_b)
{
	bool is_inside = false;
	for (const Eigen::Vector3d& v : *obj_b.get_bbox_vertices())
	{
		for (int i = 0; i < _bbox.faces.size(); ++i)
		{
			std::vector<Eigen::Vector3d> face = _bbox.faces[i];
			// get point that is on face
			Eigen::Vector3d a = face[0];
			// get vector from that point to the point that is tested
			Eigen::Vector3d av = (a - v);
			// test if this vector is one the plane <==>  <(a-v), normal> = 0
			bool is_inside_plane = false;
			if (av.dot(_bbox.normals[i]) == 0)
				is_inside_plane = true;

			if (is_inside_plane)
			{
				Eigen::Vector3d b = face[1];
				Eigen::Vector3d c = face[2];
				Eigen::Vector3d ab = (a - b);
				Eigen::Vector3d cb = (c - b);
				bool t_1 = b.dot(ab) < v.dot(ab) && v.dot(ab) < a.dot(ab);
				bool t_2 = b.dot(cb) < v.dot(cb) && v.dot(cb) < c.dot(cb);
				if (t_1 && t_2)
					is_inside = true;
			}
		}
	}

	return is_inside;
}

/*
 * count how often the lines of the bounding box of object b intersect with the this bounding box
 */
int Object::count_lines_inside_bb(std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>& obj_b_lines_bbox)
{
	int count = 0;
	for (auto& line : obj_b_lines_bbox)
	{
		Eigen::Vector3d l1 = line.first;
		Eigen::Vector3d l2 = line.second;
		auto min_max_bbox = get_min_max_bbox();
		Eigen::Vector3d b1 = min_max_bbox.first;  // smallest x,y,z
		Eigen::Vector3d b2 = min_max_bbox.second;  // biggest x,y,z
		if ((l2.x() < b1.x() && l1.x() < b1.x()) ||
			(l2.x() > b2.x() && l1.x() > b2.x()) ||
			(l2.y() < b1.y() && l1.y() < b1.y()) ||
			(l2.y() > b2.y() && l1.y() > b2.y()) ||
			(l2.z() < b1.z() && l1.z() < b1.z()) ||
			(l2.z() > b2.z() && l1.z() > b2.z()))
		{
			// no intersection possible
			continue;
		}
		if (l1.x() > b1.x() && l1.x() < b2.x() &&
			l1.y() > b1.y() && l1.y() < b2.y() &&
			l1.z() > b1.z() && l1.z() < b2.z())
		{
			// the line is fully inside the bounding box
			continue;
		}
		auto possible_interesctions = {
				get_intersection_of_line_with_bbox(l1.x() - b1.x(), l2.x() - b1.x(), l1, l2),
				get_intersection_of_line_with_bbox(l1.y() - b1.y(), l2.y() - b1.y(), l1, l2),
				get_intersection_of_line_with_bbox(l1.z() - b1.z(), l2.z() - b1.z(), l1, l2),
				get_intersection_of_line_with_bbox(l1.x() - b2.x(), l2.x() - b1.x(), l1, l2),
				get_intersection_of_line_with_bbox(l1.y() - b2.y(), l2.y() - b1.y(), l1, l2),
				get_intersection_of_line_with_bbox(l1.z() - b2.z(), l2.z() - b1.z(), l1, l2),
		};
		for (auto intersection : possible_interesctions)
		{
			if (intersection && is_inside_box(*intersection))
				count++;
		}
	}
	return count;
}

std::string Object::relation_to(Object& obj_b)
{
	// calculates the relation to obj_b based on their bounding boxes
	if (bbox_vertices_equal_to(obj_b))
		return "EQ";

	// number of points from obj_b that are inside this bounding box
	int inside_count = count_inside_bb(*obj_b.get_bbox_vertices());
	// number of intersections between lines of bbox obj_b with this bbox
	int line_interesection_count_ab = count_lines_inside_bb(*obj_b.get_bbox_edges());
	int line_interesection_count_ba = obj_b.count_lines_inside_bb(_bbox.edges);

	// check for partial intersection
	bool has_line_intersections = line_interesection_count_ab > 0 || line_interesection_count_ba > 0;
	if (has_line_intersections)
	{
		if (is_tangent_to(obj_b))
			return "EC";
		else
			return "PO";
	}

	// check obj_b contained in this bounding box
	if (inside_count == 8)
	{
		if (is_tangent_to(obj_b))
			return "TPPc";
		return "NTTPc";
	}

	// check if this is contained in obj_b
	int inside_count_c = obj_b.count_inside_bb(*get_bbox_vertices());
	if (inside_count_c == 8)
	{
		if (is_tangent_to(obj_b))
			return "TPP";
		return "NTPP";
	}

	return "DC";
}

/*
 * Calculate on which side of this object, obj_b is
 * 0 -> front
 * 1 -> up
 * 2 -> right
 * 3 -> down
 * 4 -> left
 * 5 -> behind
 */
int Object::side_of(Object& obj_b) const
{
	auto diff_centroids = *obj_b.get_centroid() - _centroid;
	// compute the angles the vector makes to the three positive axis
	double yz2_sqrt = std::sqrt(std::pow(diff_centroids.y(), 2) + std::pow(diff_centroids.z(), 2));
	auto angle_x = std::atan2(yz2_sqrt, diff_centroids.x());
	double zx2_sqrt = std::sqrt(std::pow(diff_centroids.z(), 2) + std::pow(diff_centroids.x(), 2));
	auto angle_y = std::atan2(zx2_sqrt, diff_centroids.y());
	double xy2_sqrt = std::sqrt(std::pow(diff_centroids.x(), 2) + std::pow(diff_centroids.y(), 2));
	auto angle_z = std::atan2(xy2_sqrt, diff_centroids.z());
	// convert to degrees
	angle_x *= 180 / M_PI;
	angle_y *= 180 / M_PI;
	angle_z *= 180 / M_PI;

	// project difference of centroids on each axis and get absolute value
	Eigen::Vector3d e1(1, 0, 0);
	Eigen::Vector3d e2(0, 1, 0);
	Eigen::Vector3d e3(0, 0, 1);
	auto x_len = std::abs(diff_centroids.dot(e1));
	auto y_len = std::abs(diff_centroids.dot(e2));
	auto z_len = std::abs(diff_centroids.dot(e3));

	// choose the direction in which diff_centroids "points the most"
	if (x_len >= y_len && x_len >= z_len)
	{
		if ((0 <= angle_x && angle_x < 90) || (270 <= angle_x && angle_x < 360))
			// difference of centroids points right ==> object is to the left
			std::cout << "right" << std::endl;
		else if (90 <= angle_x && angle_x < 270)
			// difference of centroids points left ==> object is to the right
			std::cout << "left" << std::endl;
	}

	if (y_len >= x_len && y_len >= z_len)
	{
		if ((0 <= angle_y && angle_y < 90) || (270 <= angle_y && angle_y < 360))
			// difference of centroids points back ==> object is in the front
			std::cout << "back" << std::endl;
		else if (90 <= angle_y && angle_y < 270)
			// difference of centroids points front ==> object is behind
			std::cout << "front" << std::endl;
	}

	if (z_len >= x_len && z_len >= y_len)
	{
		if ((0 <= angle_z && angle_z < 90) || (270 <= angle_z && angle_z < 360))
			std::cout << "up" << std::endl;
		else if (90 <= angle_z && angle_z < 270)
			std::cout << "down" << std::endl;
	}

	return -1;
}

/*
 * Calculates the OLink between this Objects Bounding box and the bounding box of obj_b
 */
std::optional<std::string> Object::intrinsic_orientation_to(Object& obj_b)
{
	bool is_smaller = _volume < obj_b.get_volume();

	if (!is_smaller)
		return std::nullopt;

	/*
	double distance = get_distance_to(obj_b);
	// we only check objects that are twice the volume away
	if (distance > _volume * 2)
		return std::nullopt;
	*/

	switch (side_of(obj_b))
	{
	case 0:
		return "in_front_of";
	case 1:
		if (is_tangent_to(obj_b))
			return "on";
		return "above";
	case 2:
		return "next_to";
	case 3:
		if (is_tangent_to(obj_b))
			return "beneath";
		return "below";
	case 4:
		return "next_to";
	case 5:
		return "behind";
	default:
		return std::nullopt;
	}
}

/*
 * Represent this Object as a SPATIAL_ENTITY in xml
 */
tinyxml2::XMLElement* Object::as_xml(tinyxml2::XMLDocument& doc)
{
	tinyxml2::XMLElement* spatial = doc.NewElement("SPATIAL_ENTITY");
	spatial->SetAttribute("objectId", _id.c_str());
	spatial->SetAttribute("label", _label.c_str());

	std::stringstream location;
	location << _centroid[0] << ", " << _centroid[1] << ", " << _centroid[2];
	spatial->SetAttribute("location", location.str().c_str());

	return spatial;
}
