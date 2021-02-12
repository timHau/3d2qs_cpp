#include <iostream>
#include "Object.h"

Object::Object(const std::shared_ptr<cpptoml::table> &obj) {
    _label = *(obj->get_qualified_as<std::string>("label"));
    _id = *(obj->get_qualified_as<std::string>("id"));
    init_bbox(obj);
}

void Object::init_bbox(const std::shared_ptr<cpptoml::table> &obj) {
    auto box = obj->get_array_of<cpptoml::array>("bbox");

    // vertices of bounding box
    for (int i = 0; i < 8; ++i) {
        cpptoml::option<std::vector<double>> point = (*box)[i]->get_array_of<double>();
        Eigen::Vector3d v((*point)[0], (*point)[1], (*point)[2]);
        _bbox.emplace_back(v);
    }

    // edges of bounding box
    std::vector<std::pair<int, int>> pairs = {
            {0, 1},
            {1, 2},
            {2, 3},
            {3, 0},
            {0, 4},
            {4, 5},
            {5, 6},
            {6, 7},
            {7, 4},
            {7, 3},
            {6, 2},
            {5, 1},
    };
    for (std::pair<int, int> p : pairs) {
        std::pair<Eigen::Vector3d, Eigen::Vector3d> line(_bbox[p.first], _bbox[p.second]);
        _bbox_lines.emplace_back(line);
    }

    // set centroid of bounding box
    Eigen::Vector3d V = _bbox[0];
    Eigen::Vector3d A = _bbox[1];
    Eigen::Vector3d B = _bbox[3];
    Eigen::Vector3d C = _bbox[4];

    _centroid = 1/2*(A + B + C - V);
}

std::vector<Eigen::Vector3d> *Object::get_bbox() {
    return &_bbox;
}

std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> *Object::get_bbox_lines() {
    return &_bbox_lines;
}

std::pair<Eigen::Vector3d, Eigen::Vector3d> Object::get_min_max_bbox() {
    // returns smallest and largest x,y,z point of the bounding box
    std::pair<Eigen::Vector3d, Eigen::Vector3d> values(_bbox[0], _bbox[6]);
    return values;
}

std::string *Object::get_label() {
    return &_label;
}

std::string *Object::get_id() {
    return &_id;
}

bool Object::is_equal_to(Object obj_b) {
    return _bbox == *obj_b.get_bbox();
}

std::optional<Eigen::Vector3d>
Object::get_intersection(double dist_1, double dist_2, const Eigen::Vector3d &p1, const Eigen::Vector3d &p2) {
    // return the point of intersection between a line and the bounding box
    if (dist_1 * dist_2 != 0 || dist_1 == dist_2) {
        return std::nullopt;
    }
    return p1 + (p2 - p1) * -(dist_1 / (dist_2 - dist_1));
}

bool Object::is_inside_box(const Eigen::Vector3d &p) {
    // get the points from bottom and top face
    std::vector<Eigen::Vector3d> bbox_a = *get_bbox();
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

int Object::count_inside_bb(std::vector<Eigen::Vector3d> &obj_b_bbox) {
    // test which points from obj_b bounding box is inside this bounding box, returns indices

    // number of points in obj_bs bounding box that are inside obj_as bounding box
    int count = 0;
    for (const Eigen::Vector3d &P : obj_b_bbox) {
        if (is_inside_box(P)) {
            count++;
        }
    }
    return count;
}

bool Object::is_tangent_to(Object &obj_b) {
    // test if obj_b bounding box is inside the face of this bounding box

    std::vector<Eigen::Vector3d> bbox_a = *get_bbox();
    const std::vector<std::vector<Eigen::Vector3d>> faces = {
            {bbox_a[0], bbox_a[1], bbox_a[2], bbox_a[3]},  // vorne
            {bbox_a[3], bbox_a[2], bbox_a[6], bbox_a[7]},  // oben
            {bbox_a[1], bbox_a[5], bbox_a[6], bbox_a[2]},  // rechts
            {bbox_a[4], bbox_a[5], bbox_a[1], bbox_a[0]},  // unten
            {bbox_a[4], bbox_a[0], bbox_a[3], bbox_a[7]},  // links
            {bbox_a[5], bbox_a[4], bbox_a[7], bbox_a[6]},  // hinten
    };

    std::vector<Eigen::Vector3d> normals = {
            (bbox_a[2] - bbox_a[1]).cross(bbox_a[0] - bbox_a[1]),
            (bbox_a[6] - bbox_a[5]).cross(bbox_a[1] - bbox_a[5]),
            (bbox_a[6] - bbox_a[2]).cross(bbox_a[3] - bbox_a[2]),
            (bbox_a[0] - bbox_a[1]).cross(bbox_a[5] - bbox_a[1]),
            (bbox_a[3] - bbox_a[0]).cross(bbox_a[4] - bbox_a[0]),
            (bbox_a[7] - bbox_a[4]).cross(bbox_a[5] - bbox_a[4]),
    };
    for (Eigen::Vector3d &normal : normals) {
        normal = normal.normalized();
    }

    bool is_inside = false;
    for (const Eigen::Vector3d &v : *obj_b.get_bbox()) {
        for (int i = 0; i < faces.size(); ++i) {
            std::vector<Eigen::Vector3d> face = faces[i];
            // get point that is on face
            Eigen::Vector3d a = face[0];
            // get vector from that point to the point that is tested
            Eigen::Vector3d av = (a - v);
            // test if this vector is one the plane <==>  <(a-v), normal> = 0
            bool is_inside_plane = false;
            if (av.dot(normals[i]) == 0) {
                is_inside_plane = true;
            }

            if (is_inside_plane) {
                Eigen::Vector3d b = face[1];
                Eigen::Vector3d c = face[2];
                Eigen::Vector3d ab = (a - b);
                Eigen::Vector3d cb = (c - b);
                bool t_1 = b.dot(ab) < v.dot(ab) && v.dot(ab) < a.dot(ab);
                bool t_2 = b.dot(cb) < v.dot(cb) && v.dot(cb) < c.dot(cb);
                if (t_1 && t_2) {
                    is_inside = true;
                }
            }
        }
    }

    return is_inside;
}

int Object::count_lines_inside_bb(std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> &obj_b_lines_bbox) {
    // count how often the lines of the bounding box of object b intersect with the this bounding box
    int count = 0;
    for (auto &line : obj_b_lines_bbox) {
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
            (l2.z() > b2.z() && l1.z() > b2.z())) {
            // no intersection possible
            continue;
        }
        if (l1.x() > b1.x() && l1.x() < b2.x() &&
            l1.y() > b1.y() && l1.y() < b2.y() &&
            l1.z() > b1.z() && l1.z() < b2.z()) {
            // the line is fully inside the box, we count that as a intersection
            count++;
            continue;
        }
        auto possible_interesctions = {
                get_intersection(l1.x() - b1.x(), l2.x() - b1.x(), l1, l2),
                get_intersection(l1.y() - b1.y(), l2.y() - b1.y(), l1, l2),
                get_intersection(l1.z() - b1.z(), l2.z() - b1.z(), l1, l2),
                get_intersection(l1.x() - b2.x(), l2.x() - b1.x(), l1, l2),
                get_intersection(l1.y() - b2.y(), l2.y() - b1.y(), l1, l2),
                get_intersection(l1.z() - b2.z(), l2.z() - b1.z(), l1, l2),
        };
        for (auto &intersection : possible_interesctions) {
            if (intersection && is_inside_box(*intersection)) {
                count++;
            }
        }
    }
    return count;
}

std::string Object::relation_to(Object obj_b) {
    // calculates the relation to obj_b based on their bounding boxes
    if (is_equal_to(obj_b)) {
        return "EQ";
    }

    // number of points from obj_b that are inside this bounding box
    int inside_count = count_inside_bb(*obj_b.get_bbox());
    // number of intersections between lines of bbox obj_b with this bbox
    int count_line_interesctions = count_lines_inside_bb(*obj_b.get_bbox_lines());

    // check for partial intersection
    if ((0 < inside_count && inside_count < 8) || count_line_interesctions > 0) {
        if (is_tangent_to(obj_b)) {
            return "EC";
        } else {
            return "PO";
        }
    }

    // check disjoint
    if (inside_count == 0) {
        return "DC";
    }

    // check obj_b contained in this bounding box
    if (inside_count == 8) {
        if (is_tangent_to(obj_b)) {
            return "TPPc";
        }
        return "NTTPc";
    }

    // check if this is contained in obj_b
    int inside_count_c = obj_b.count_inside_bb(*get_bbox());
    if (inside_count_c == 8) {
        if (is_tangent_to(obj_b)) {
            return "TPP";
        }
        return "NTPP";
    }

    return std::string();
}

tinyxml2::XMLElement* Object::as_xml(tinyxml2::XMLDocument &doc) {
    tinyxml2::XMLElement *spatial = doc.NewElement("SPATIAL_ENTITY");
    spatial->SetAttribute("ObjectID", _id.c_str());
    return spatial;
}
