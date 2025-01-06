#ifndef STEINER_METHODS_HPP
#define STEINER_METHODS_HPP

#include "../custom_cdt_class/custom_cdt_class.hpp"
#include "../utils/utils.hpp"
#include "../read_write_file/read_write_file.hpp"
#include <random>
#include <CGAL/Random.h>

typedef CGAL::Constrained_triangulation_plus_2<custom_cdt_class::Custom_Constrained_Delaunay_triangulation_2<K, CGAL:: Default, Itag>> CDT;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef K::Segment_2 Segment;
typedef CDT::Point Point;
typedef CDT::Edge Edge;

using namespace utils;
using namespace read_write_file;

using SteinerMethodObtPoint = obt_point (*)(CDT&, CDT::Face_handle);
using SteinerMethodObtFace = obt_face (*)(CDT&, FaceData);

namespace steiner_methods {
    enum class InsertionMethod {
        PROJECTION,
        MIDPOINT,
        CENTROID,
        CIRCUMCENTER,
        MERGE_OBTUSE,
        NONE
    };

    // Choose a random Steiner method
    InsertionMethod choose_random_steiner_method(AvailableSteinerMethods available_steiner_methods);

    // Insert the projection point of the obtuse vertex onto the opposite edge
    obt_point insert_projection(CDT& cdt, CDT::Face_handle f1);

    // Insert a point at the centroid of the triangle
    obt_point insert_centroid(CDT& cdt, CDT::Face_handle f1);

    // Insert a point at the midpoint of the longest edge
    obt_point insert_mid(CDT& cdt, CDT::Face_handle f1);

    obt_point insert_random(CDT& cdt);

    // Insert a point at the circumcenter of the triangle
    obt_face insert_circumcenter(CDT& cdt, FaceData f1);

    // Merge triangles if possible
    obt_face merge_obtuse(CDT& cdt, FaceData f1);
}

#endif // STEINER_METHODS_HPP
