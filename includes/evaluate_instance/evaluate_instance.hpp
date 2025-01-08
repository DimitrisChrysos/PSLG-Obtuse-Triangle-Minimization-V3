#ifndef EVALUATE_INSTANCE_HPP
#define EVALUATE_INSTANCE_HPP

#include "../custom_cdt_class/custom_cdt_class.hpp"
#include "../utils/utils.hpp"

#define ALPHA 3
#define BETA 0.3

typedef CGAL::Constrained_triangulation_plus_2<custom_cdt_class::Custom_Constrained_Delaunay_triangulation_2<K, CGAL:: Default, Itag>> CDT;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef K::Segment_2 Segment;
typedef CDT::Point Point;
typedef CDT::Edge Edge;

using namespace utils;

namespace evaluate_instance {

    extern double sum_of_rates_of_convergence;

    double calculate_average_rate_of_convergence(int total_number_of_steiner_points);

    void calculate_rate_of_convergence(CDT& cdt_with_new_steiner, int old_obtuse_count, int old_steiner_count);

    double calculate_energy(int obtuse_count, int steiner_count);
    
    bool file_exists(const std::string& file_name);

    void for_test_instances_dot_py(int argc, char *argv[], CDT& cdt);
}

#endif // EVALUATE_INSTANCE_HPP