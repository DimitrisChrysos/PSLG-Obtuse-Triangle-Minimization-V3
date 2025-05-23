#ifndef READ_WRITE_FILE_HPP
#define READ_WRITE_FILE_HPP

#include "../custom_cdt_class/custom_cdt_class.hpp"
#include "../utils/utils.hpp"
#include "../input_categories/input_categories.hpp"
#include <boost/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

typedef CGAL::Constrained_triangulation_plus_2<custom_cdt_class::Custom_Constrained_Delaunay_triangulation_2<K, CGAL:: Default, Itag>> CDT;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef K::Segment_2 Segment;
typedef CDT::Point Point;
typedef CDT::Edge Edge;

using namespace utils;
using namespace input_categories;

namespace read_write_file {
    class AvailableSteinerMethods {
        public:
            bool proj;
            bool centr;
            bool mid;
            bool circum;
            bool merge;

            AvailableSteinerMethods(bool proj, bool centr, bool mid, bool circum, bool merge) 
            : proj(proj), centr(centr), mid(mid), circum(circum), merge(merge) {}
    };

    // Read JSON functions
    std::string get_instance_uid(boost::property_tree::ptree root);

    int get_num_points(boost::property_tree::ptree root);

    std::list<int> get_points_x(boost::property_tree::ptree root);

    std::list<int> get_points_y(boost::property_tree::ptree root);

    std::list<int> get_region_boundary(boost::property_tree::ptree root);

    std::string get_num_constraints(boost::property_tree::ptree root);

    std::list<std::pair<int, int>> get_additional_constraints(boost::property_tree::ptree root);

    std::list<std::pair<int, int>> combine_constraints(std::list<int> region_boundary, std::list<std::pair<int, int>> additional_constraints);

    std::string get_method(boost::property_tree::ptree root);

    std::list<std::pair<std::string, double>> get_parameters(boost::property_tree::ptree root);

    bool get_delaunay(boost::property_tree::ptree root);

    // For the auto functionality
    void choose_auto(std::string& method, 
                    std::list<std::pair<std::string, double>>& parameters, 
                    boost::property_tree::ptree& parameters_for_output, 
                    AvailableSteinerMethods& available_steiner_methods,
                    InputCategory input_category);

    // Scan the configuration from the argv arguments
    void scan_config(int argc, char *argv[], 
                    boost::property_tree::ptree& root, 
                    std::string& method, 
                    std::list<std::pair<std::string, double>>& parameters, 
                    boost::property_tree::ptree& parameters_for_output, 
                    bool& delaunay, 
                    AvailableSteinerMethods& available_steiner_methods,
                    InputCategory input_category);

    // Create the output file
    void write_output(CDT& cdt, 
                        std::vector<Point> points, 
                        std::string method, 
                        boost::property_tree::ptree parameters_for_output,
                        const std::string& output_file);
}


#endif // READ_WRITE_FILE_HPP