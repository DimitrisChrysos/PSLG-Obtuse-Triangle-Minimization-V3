#include "read_write_file.hpp"
#include <boost/json/src.hpp>
#include <filesystem>

// Read JSON functions
std::string read_write_file::get_instance_uid(boost::property_tree::ptree root) {
  return root.get<std::string>("instance_uid");
}

int read_write_file::get_num_points(boost::property_tree::ptree root) {
  return root.get<int>("num_points", 0);;
}

std::list<int> read_write_file::get_points_x(boost::property_tree::ptree root) {
  std::list<int> points_x;
  for (boost::property_tree::ptree::value_type &point_x : root.get_child("points_x")) {
    points_x.push_back(point_x.second.get_value<int>());
  }
  return points_x;
}

std::list<int> read_write_file::get_points_y(boost::property_tree::ptree root) {
  std::list<int> points_y;
  for (boost::property_tree::ptree::value_type &point_y : root.get_child("points_y")) {
    points_y.push_back(point_y.second.get_value<int>());
  }
  return points_y;
}

std::list<int> read_write_file::get_region_boundary(boost::property_tree::ptree root) {
  std::list<int> region_boundary;
  for (boost::property_tree::ptree::value_type &temp : root.get_child("region_boundary")) {
    region_boundary.push_back(temp.second.get_value<int>());
  }
  return region_boundary;
}

std::string read_write_file::get_num_constraints(boost::property_tree::ptree root) {
  return root.get<std::string>("num_constraints");
}

std::list<std::pair<int, int>> read_write_file::get_additional_constraints(boost::property_tree::ptree root) {
  std::list<std::pair<int, int>> additional_constraints;
  for (boost::property_tree::ptree::value_type &row : root.get_child("additional_constraints")) {
    auto it = row.second.begin();
    int first = it->second.get_value<int>();
    ++it;
    int second = it->second.get_value<int>();
    additional_constraints.push_back(std::make_pair(first, second));
  }
  return additional_constraints;
}

std::list<std::pair<int, int>> read_write_file::combine_constraints(std::list<int> region_boundary, std::list<std::pair<int, int>> additional_constraints) {
  // Add region_boundary to additional_constraints
  int prev = -1;
  int first;
  for (const auto &temp : region_boundary) {
    if (prev == -1) {
      prev = temp;
      first = temp;
      continue;
    }
    additional_constraints.push_back(std::make_pair(prev, temp));
    prev = temp;
  }
  additional_constraints.push_back(std::make_pair(prev, first));

  return additional_constraints;
}

std::string read_write_file::get_method(boost::property_tree::ptree root) {
  return root.get<std::string>("method");
}

std::list<std::pair<std::string, double>> read_write_file::get_parameters(boost::property_tree::ptree root) {
  std::list<std::pair<std::string, double>> parameters;
  for (boost::property_tree::ptree::value_type &row : root.get_child("parameters")) {
    std::string first = row.first;
    double second = row.second.get_value<double>();

    parameters.push_back(std::make_pair(first, second));
  }

  return parameters;
}

bool read_write_file::get_delaunay(boost::property_tree::ptree root) {
  return root.get<bool>("delaunay");
}


std::string return_frac_string(const CGAL::Epeck::FT &x) {

  const auto exact_coord = CGAL::exact(x);

  // Convert the exact coordinate to a GMP rational (mpq_t)
  const mpq_t* gmpq_ptr = reinterpret_cast<const mpq_t*>(&exact_coord);

  // Declare GMP integers to hold the numerator and denominator
  mpz_t num, den;
  mpz_init(num);
  mpz_init(den);

  // Extract the numerator and denominator using GMP functions
  mpq_get_num(num, *gmpq_ptr);
  mpq_get_den(den, *gmpq_ptr);


  size_t buffer_size = mpz_sizeinbase(num, 10) + mpz_sizeinbase(den, 10) + 2;
  std::string frac(buffer_size, '\0');
  int written = __gmp_snprintf(&frac[0], buffer_size, "%Zd/%Zd", num, den);
  if (written >= 0) {
    frac.resize(written);
  }

  mpz_clear(num);
  mpz_clear(den);

  return frac;
}

boost::json::object ptree_to_json(const boost::property_tree::ptree& pt) {
  boost::json::object obj;
  for (const auto& item : pt) {
      obj[item.first] = item.second.get_value<double>();
  }
  return obj;
}

// For the auto functionality
void read_write_file::choose_auto(std::string& method, 
                    std::list<std::pair<std::string, double>>& parameters, 
                    boost::property_tree::ptree& parameters_for_output, 
                    AvailableSteinerMethods& available_steiner_methods) {

  ///TODO: This needs proper implementation
  std::cout << "Using preselected parameters\n";  // for auto (preselected params)
  method = "local";
  parameters = {{"L", 90}};
  parameters_for_output.put("L", 90);
  available_steiner_methods = {true, true, true, true, true};
}

// Scan the configuration from the argv arguments
void read_write_file::scan_config(int argc, char *argv[], 
                    boost::property_tree::ptree& root, 
                    std::string& method, 
                    std::list<std::pair<std::string, double>>& parameters, 
                    boost::property_tree::ptree& parameters_for_output, 
                    bool& delaunay, 
                    AvailableSteinerMethods& available_steiner_methods) {

  if (argc == 5) {
    method = get_method(root);
    parameters = get_parameters(root);
    parameters_for_output = root.get_child("parameters");
    delaunay = get_delaunay(root);
    available_steiner_methods = {true, true, true, true, true};
  }
  else {
    if (strcmp(argv[5], "-preselected_params") == 0) {
      choose_auto(method, parameters, parameters_for_output, available_steiner_methods);
    }
    else {
      if (strcmp(argv[5], "-ls") == 0) {
        method = "local";
        parameters = {{"L", 90}};
        parameters_for_output.put("L", 90);
      } 
      else if (strcmp(argv[5], "-sa") == 0) {
        method = "sa";
        parameters = {{"alpha", 3}, {"beta", 0.3}, {"L", 200}};
        parameters_for_output.put("alpha", 3);
        parameters_for_output.put("beta", 0.3);
        parameters_for_output.put("L", 200);
      }
      else if (strcmp(argv[5], "-ant") == 0) {
        method = "ant";
        parameters = {{"alpha", 3}, {"beta", 0.3}, 
                      {"xi", 1.0}, {"psi", 3.0}, 
                      {"lambda", 0.7}, {"kappa", 10}, {"L", 30}};
        parameters_for_output.put("alpha", 3);
        parameters_for_output.put("beta", 0.3);
        parameters_for_output.put("xi", 1.0);
        parameters_for_output.put("psi", 3.0);
        parameters_for_output.put("lambda", 0.7);
        parameters_for_output.put("kappa", 10);
        parameters_for_output.put("L", 30);
      } 
      else {
        std::cout << "Error: Invalid method from arguments, use -ls, -sa or -ant \n";
        exit (EXIT_FAILURE);
      }

      if (argc == 6) {
        std::cout << "Error: You need to provide available steiner methods, use -proj, -mid, -centr, -circum or -merge\n";
        exit (EXIT_FAILURE);
      }

      for (int i = 6; i < argc; i++) {
        if (strcmp(argv[i], "-proj") == 0) available_steiner_methods.proj = true;
        else if (strcmp(argv[i], "-centr") == 0) available_steiner_methods.centr = true;
        else if (strcmp(argv[i], "-mid") == 0) available_steiner_methods.mid = true;
        else if (strcmp(argv[i], "-circum") == 0) available_steiner_methods.circum = true;
        else if (strcmp(argv[i], "-merge") == 0) available_steiner_methods.merge = true;
        else {
          std::cout << "Error: Invalid steiner method from arguments, use -proj, -mid, -centr, -circum or -merge\n";
          exit (EXIT_FAILURE);
        }
      }
    }
    delaunay = true; // Delaunay is always true in this case
  }
}

// Create the output file
void read_write_file::write_output(CDT& cdt, 
                                    std::vector<Point> points, 
                                    std::string method, 
                                    boost::property_tree::ptree parameters_for_output,
                                    const std::string& output_file) {
  boost::json::object root1;
  root1["content_type"] = "CG_SHOP_2025_Solution";
  root1["instance_uid"] = "some_instance_uid";

  // JSON array to hold steiner_x values
  boost::json::array steiner_x_json;
  for (const auto& x : cdt.steiner_x) {
    double x1 = static_cast<double>(CGAL::to_double(x));
    // If value is an int
    if (x1 == static_cast<double>(static_cast<int>(x1))) {
      int x11 = static_cast<int>(x1);
      steiner_x_json.push_back(boost::json::value(std::to_string(x11)));
    }
    else {
      steiner_x_json.push_back(boost::json::value(return_frac_string(x)));
    }
  }

  // Add steiner_x JSON array to root object
  root1["steiner_points_x"] = steiner_x_json;

  // JSON array to hold steiner_y values
  boost::json::array steiner_y_json;
  for (const auto& y : cdt.steiner_y) {
    double y1 = static_cast<double>(CGAL::to_double(y));
    if (y1 == static_cast<double>(static_cast<int>(y1))) {
      int y11 = static_cast<int>(y1);
      steiner_y_json.push_back(boost::json::value(std::to_string(y11)));
    }
    else {
      steiner_y_json.push_back(boost::json::value(return_frac_string(y)));
    }
  }

  // Add steiner_x JSON array to root object
  root1["steiner_points_y"] = steiner_y_json;

  boost::json::array edges_array;

  for (CDT::Finite_edges_iterator e = cdt.finite_edges_begin(); e != cdt.finite_edges_end(); e++) {
    CDT::Face_handle f1 = e->first;
    int edge_index = e->second;

    CDT::Vertex_handle v1 = f1->vertex((edge_index + 1) % 3);
    CDT::Vertex_handle v2 = f1->vertex((edge_index + 2) % 3);

    Point p1 = v1->point();
    Point p2 = v2->point();

    int ind_1;
    int ind_2;
    bool found_1 = false;
    bool found_2 = false;

    // Traverse the points and find which 2 indexes are contained in the edge
    for (std::size_t i = 0; i < points.size(); ++i) {
      // If you find the first point
      if (p1.x() == points[i].x() && p1.y() == points[i].y()) {
        ind_1 = i;
        found_1 = true;
      }
      if (p2.x() == points[i].x() && p2.y() == points[i].y()) {
        ind_2 = i;
        found_2 = true;
      }
      if (found_1 && found_2) break;
    }

    // If you haven't found both points of the edge, continue with the Steiner points
    if (!found_1 || !found_2) {
      for (std::size_t i = 0; i < cdt.steiner_x.size(); ++i) {
        if (p1.x() == cdt.steiner_x[i] && p1.y() == cdt.steiner_y[i]) {
          // Continue the indexing after the regular points
          ind_1 = points.size() + i;
          found_1 = true;
        }
        if (p2.x() == cdt.steiner_x[i] && p2.y() == cdt.steiner_y[i]) {
          ind_2 = points.size() + i;
          found_2 = true;
        }
        if (found_1 && found_2) break;
      }
    }

    boost::json::array edge_pair;
    edge_pair.push_back(ind_1);
    edge_pair.push_back(ind_2);

    // Add the edge pair to the edges_array
    edges_array.push_back(edge_pair);
  }
  root1["edges"] = edges_array;

  // Add obtuse count to root object
  root1["obtuse_count"] = count_obtuse_triangles(cdt);

  // Add method to root object
  root1["method"] = method;

  // Add parameters to root object  
  boost::json::object parameters_json = ptree_to_json(parameters_for_output);

  // Add the JSON object to the root object
  root1["parameters"] = parameters_json;

  root1["randomization"] = cdt.random_used;

  // Serialize the whole JSON object
  std::string json_string = boost::json::serialize(root1);

  // Make the string pretty and ready to print
  std::string pretty_json;
  int indent = 0;
  char prev;
  bool in_string = false;  // Track if we are in a string
  bool in_steiner = false; // Track if we are printing Steiner points or not

  for (char ch : json_string) {
    if (ch == '"') in_string = !in_string;
    if (!in_string) {
      if (ch == ',') {
        if (prev == '"' || prev == ']') {
          pretty_json += ch;
          if (in_steiner) {
            pretty_json += ' ';
          }
          else {
            pretty_json += '\n';
          }
          prev = ch;
          continue;
        }
      }
      else if (ch == ':') {
        pretty_json += ch;
        pretty_json += ' ';
        prev = ch;
        continue;
      }
      else if (ch == '[') {
        in_steiner = true;
        if (prev == '[') {
          in_steiner = false;
          pretty_json += '\n';
        }
        pretty_json += '\t';
      }
      else if (ch == ']') {
        in_steiner = false;
      }
    }
    pretty_json += ch;
    prev = ch;
  }
  
  // Ensure the directory exists
  std::filesystem::path output_path(output_file);
  std::filesystem::path directory = output_path.parent_path();

  if (!directory.empty() && !std::filesystem::exists(directory)) {
      std::filesystem::create_directories(directory);
  }

  // Write to file
  std::ofstream file(output_file);
  file << pretty_json;
  file.close();
}