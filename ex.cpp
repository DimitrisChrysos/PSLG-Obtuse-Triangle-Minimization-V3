#include "includes/custom_cdt_class/custom_cdt_class.hpp"
#include "includes/utils/utils.hpp"
#include "includes/read_write_file/read_write_file.hpp"
#include "includes/steiner_methods/steiner_methods.hpp"
#include <random>
#include <chrono>

typedef CGAL::Constrained_triangulation_plus_2<custom_cdt_class::Custom_Constrained_Delaunay_triangulation_2<K, CGAL:: Default, Itag>> CDT;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef K::Segment_2 Segment;
typedef CDT::Point Point;
typedef CDT::Edge Edge;

using namespace utils;
using namespace steiner_methods;

// An ant is stored using the following class
class effective_ant {
  static int ant_id_counter;
  public:
    int id;
    int obt_count;
    int steiner_count;
    InsertionMethod sp_method;
    std::list<FaceData> affected_faces;
    Point insrt_pt;
    FaceData face_for_sp_method = FaceData(Point(0,0), Point(0,0), Point(0,0));

    effective_ant(int obt_count, int steiner_count, InsertionMethod sp_method, std::list<FaceData> affected_faces)
      : id(ant_id_counter++), obt_count(obt_count), steiner_count(steiner_count), sp_method(sp_method), affected_faces(affected_faces) {}

    bool operator==(const effective_ant& other) const {
      return id == other.id;
    }
};
int effective_ant::ant_id_counter = 0;

// Choose a Steiner method
InsertionMethod choose_steiner_method(CDT& cdt, CDT::Face_handle face, double k, double xi, double psi, t_sp tsp) {

  // Calculate the radius-to-height ratio
  double r_to_h = calculate_r_to_h(face);

  // Heuristics
  double h_projection = std::max(0.0 , (double)(r_to_h - (double)1) / r_to_h );
  double h_circumcenter = (double) r_to_h / (double)((double)2 + r_to_h);
  double h_midpoint = std::max(0.0, ((double)3 - (double)((double)2 * r_to_h) ) / (double)3 );
  double h_merge_obtuse = more_or_equal_to_2_adjacent_obtuse_faces(cdt, face) ? 1 : 0;

  // Posibilities
  double sum = pow(tsp.projection, xi)*pow(h_projection, psi) + 
                pow(tsp.circumcenter, xi)*pow(h_circumcenter, psi) + 
                pow(tsp.midpoint, xi)*pow(h_midpoint, psi) + 
                pow(tsp.merge_obtuse, xi)*pow(h_merge_obtuse, psi);
  double p_projection = calculate_posibility(tsp.projection, h_projection, xi, psi, sum);
  double p_circumcenter = calculate_posibility(tsp.circumcenter, h_circumcenter, xi, psi, sum);
  double p_midpoint = calculate_posibility(tsp.midpoint, h_midpoint, xi, psi, sum);
  double p_merge_obtuse = calculate_posibility(tsp.merge_obtuse, h_merge_obtuse, xi, psi, sum);

  // Choose the method
  double sum_of_probabilities = p_projection + p_circumcenter + p_midpoint + p_merge_obtuse;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, sum_of_probabilities);
  double random_number = dis(gen);
  if (random_number <= p_projection) {
    return InsertionMethod::PROJECTION;
  }
  else if (random_number <= p_projection + p_circumcenter) {
    return InsertionMethod::CIRCUMCENTER;
  }
  else if (random_number <= p_projection + p_circumcenter + p_midpoint) {
    return InsertionMethod::MIDPOINT;
  }
  else {
    return InsertionMethod::MERGE_OBTUSE;
  }
}

// Improve the triangulation
effective_ant improve_trianglulation(CDT& cdt, double k, ant_parameters ant_params, t_sp tsp) {

  // Get a random face
  std::list<CDT::Face_handle> obtuse_faces;
  for (CDT::Finite_faces_iterator f = cdt.finite_faces_begin(); f != cdt.finite_faces_end(); f++) {
    if (is_triangle_inside_region_boundary(f) && has_obtuse_angle(f)) {
      obtuse_faces.push_back(f);
    }
  }
  if (obtuse_faces.empty()) { // Fails because the triangluation is already optimal
    return effective_ant(0, 0, InsertionMethod::NONE, std::list<FaceData>());
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, obtuse_faces.size() - 1);
  int random_index = dis(gen);
  auto it = std::next(obtuse_faces.begin(), random_index);
  CDT::Face_handle random_face = *it;

  // Choose a steiner method:
  InsertionMethod steiner_method = choose_steiner_method(cdt, random_face, k, ant_params.xi, ant_params.psi, tsp);
  SteinerMethodObtPoint method_point;
  SteinerMethodObtFace method_face;
  if (steiner_method == InsertionMethod::PROJECTION) method_point = insert_projection;
  else if (steiner_method == InsertionMethod::MIDPOINT) method_point = insert_mid;
  else if (steiner_method == InsertionMethod::CENTROID) method_point = insert_centroid;
  else if (steiner_method == InsertionMethod::CIRCUMCENTER) method_face = insert_circumcenter;
  else if (steiner_method == InsertionMethod::MERGE_OBTUSE) method_face = merge_obtuse;

  // If the choosen method fails, pick the centroid method
  if (steiner_method == InsertionMethod::CIRCUMCENTER || steiner_method == InsertionMethod::MERGE_OBTUSE) {
    CDT copy(cdt);
    FaceData face_data(random_face->vertex(0)->point(), random_face->vertex(1)->point(), random_face->vertex(2)->point());
    obt_face temp = method_face(copy, face_data);
    if (temp.obt_count == 9999) {
      steiner_method = InsertionMethod::CENTROID;
      method_point = insert_centroid;
    }
  }

  // Use the method
  if (steiner_method == InsertionMethod::PROJECTION || 
      steiner_method == InsertionMethod::MIDPOINT || 
      steiner_method == InsertionMethod::CENTROID) {
    CDT save_cdt(cdt);
    obt_point returned = method_point(cdt, random_face);
    CDT::Face_handle located_face = save_cdt.locate(returned.insrt_pt);
    std::list<FaceData> affected_faces;
    store_face_data(located_face, affected_faces);
    effective_ant returned_ant(count_obtuse_triangles(cdt), 0, steiner_method, affected_faces);
    returned_ant.insrt_pt = returned.insrt_pt;
    return returned_ant;
  }
  else if (steiner_method == InsertionMethod::CIRCUMCENTER || 
           steiner_method == InsertionMethod::MERGE_OBTUSE) {
    FaceData face_data_final(random_face->vertex(0)->point(), random_face->vertex(1)->point(), random_face->vertex(2)->point());
    obt_face returned = method_face(cdt, face_data_final);
    std::list<FaceData> affected_faces;
    for (CDT::Face_handle face : returned.affected_faces) {
      store_face_data(face, affected_faces);
    }
    effective_ant returned_ant(count_obtuse_triangles(cdt), 0, steiner_method, affected_faces);
    FaceData face_data(returned.face->vertex(0)->point(), returned.face->vertex(1)->point(), returned.face->vertex(2)->point());
    returned_ant.face_for_sp_method = face_data;
  return returned_ant;
  }

  return effective_ant(9999, 0, InsertionMethod::NONE, std::list<FaceData>());
}

// Evaluate the triangulation
bool evaluate_trianguation(CDT& cdt, int before_cycle_obt_count, InsertionMethod steiner_method, dt& Dt, int steiner_counter, ant_parameters ant_params) {
  int obt_count = count_obtuse_triangles(cdt);
  if (obt_count < before_cycle_obt_count) {
    if (steiner_method == InsertionMethod::PROJECTION) Dt.projection = update_dt(obt_count, Dt.projection, steiner_counter, ant_params);
    else if (steiner_method == InsertionMethod::MIDPOINT) Dt.midpoint = update_dt(obt_count, Dt.midpoint, steiner_counter, ant_params);
    else if (steiner_method == InsertionMethod::CENTROID) Dt.centroid = update_dt(obt_count, Dt.centroid, steiner_counter, ant_params);
    else if (steiner_method == InsertionMethod::CIRCUMCENTER) Dt.circumcenter = update_dt(obt_count, Dt.circumcenter, steiner_counter, ant_params);
    else if (steiner_method == InsertionMethod::MERGE_OBTUSE) Dt.merge_obtuse = update_dt(obt_count, Dt.merge_obtuse, steiner_counter, ant_params);
    return true;
  }
  return false;
}

// Handles conflicts, returns true if it encounters a conflict
bool handle_conflicts(CDT &cdt, std::list<effective_ant>& effective_ants, effective_ant& new_ant) {
  for (effective_ant& ant : effective_ants) {
    for (FaceData face1 : ant.affected_faces) {
      for (FaceData face2 : new_ant.affected_faces) {
        if (same_faces(face1, face2)) {
          if (ant.obt_count > new_ant.obt_count) {
            effective_ants.remove(ant);
            effective_ants.push_back(new_ant);
          }
          return true;
        }
      }
    }
  }
  new_ant.steiner_count += 1;
  effective_ants.push_back(new_ant);
  return false;
}

// Use the triangulation ants to the cdt
void use_triangulation_ants(CDT& cdt, std::list<effective_ant>& ants) {
  for (effective_ant& ant : ants) {
    InsertionMethod method = ant.sp_method;
    if (method == InsertionMethod::PROJECTION || 
        method == InsertionMethod::MIDPOINT || 
        method == InsertionMethod::CENTROID) {
      cdt.insert_no_flip(ant.insrt_pt);
      cdt.insert_steiner_x_y(ant.insrt_pt.x(), ant.insrt_pt.y());
    }
    else if (method == InsertionMethod::CIRCUMCENTER) {
      insert_circumcenter(cdt, ant.face_for_sp_method);
    }
    else if (method == InsertionMethod::MERGE_OBTUSE) {
      merge_obtuse(cdt, ant.face_for_sp_method);
    }
  }
}

// Check to find the best triangulation, returns true if the triangulation is optimal
bool save_best_triangulation(CDT& cdt, std::list<effective_ant>& effective_ants, 
                              ant_parameters ant_params, double& starting_energy,
                              std::list<effective_ant>& best_triangulation_ants) {
  CDT copy(cdt);
  use_triangulation_ants(copy, effective_ants);
  std::cout << "|| obtuse: " << count_obtuse_triangles(copy) << " | Steiner: " << effective_ants.size() << std::endl;
  if (count_obtuse_triangles(copy) == 0) {
    best_triangulation_ants = effective_ants;
    return true;
  }
  int steiner_counter = effective_ants.size();
  double current_energy = ant_params.alpha * count_obtuse_triangles(copy) + ant_params.beta * steiner_counter;
  double de = current_energy - starting_energy;
  if (de < 0) {
    best_triangulation_ants = effective_ants;
    starting_energy = current_energy;
  }
  return false;
}

// The ant colony optimization algorithm
void ant_colony_optimization(CDT& cdt, ant_parameters ant_params) {

  std::cout<< "L: " << ant_params.L << " | Kappa: " << ant_params.kappa << std::endl;

  if (count_obtuse_triangles(cdt) == 0) {
    std::cout << "\nFinal -> Obtuse Triangles: 0 || Steiner Points: 0" << std::endl; 
    return;
  }

  t_sp tsp(0.5, 0.5, 0.5, 0.5, 0.5);
  std::list<effective_ant> best_triangulation_ants;
  double starting_energy = ant_params.alpha * count_obtuse_triangles(cdt) + ant_params.beta * 0;
  for (int c = 1 ; c <= ant_params.L ; c++) {  // c -> cycle

    dt Dt(0, 0, 0, 0, 0);
    int before_cycle_obt_count = count_obtuse_triangles(cdt);
    std::list<effective_ant> effective_ants;

    for (int k = 1 ; k <= ant_params.kappa ; k++) { // k -> ant

      CDT copy(cdt);

      // Improve the triangulation
      effective_ant new_ant = improve_trianglulation(copy, k, ant_params, tsp);
      if (new_ant.obt_count == 0) { // If the triangulation is optimal
        break;
      }
      new_ant.steiner_count = effective_ants.size();

      // Handle conflicts and Evaluate the triangulation
      handle_conflicts(copy, effective_ants, new_ant);
      evaluate_trianguation(copy, before_cycle_obt_count, new_ant.sp_method, Dt, effective_ants.size(), ant_params);
    }

    std::cout << "Cycle: " << c << " finished with -> " << "|| effective ants: " << effective_ants.size();

    // SaveBestTriangulation(c)
    if (save_best_triangulation(cdt, effective_ants, ant_params, starting_energy, best_triangulation_ants)) {
      break; // Break because the triangulation is optimal
    }

    // UpdatePheromones(c)
    update_pheromones(tsp, ant_params, Dt);
  }

  // Use the best triangulation to the starting cdt
  use_triangulation_ants(cdt, best_triangulation_ants);
  std::cout << "\nFinal -> Obtuse Triangles: " << count_obtuse_triangles(cdt) << " || Steiner Points: " << best_triangulation_ants.size() << std::endl;
}

// Accept or decline something with the given probability
bool accept_or_decline(double prob) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::bernoulli_distribution dist(prob);
  return dist(gen);
}

// The simulated annealing algorithm
void sim_annealing(CDT& cdt, double a, double b, int L) {
  int steiner_counter = 0;
  double cur_en = a * count_obtuse_triangles(cdt) + b * steiner_counter;
  double T = 1;
  double new_en, de;
  double e = std::exp(1);
  int tries = 0;
  while (T >= 0) {
    
    tries++;
    bool flag = true;

    while (flag) {
      flag = false;

      if (T < 0) {
        break;
      }

      // Iterate the faces of the cdt
      for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); fit++) {

        CDT::Face_handle f = fit;

        if (!is_triangle_inside_region_boundary(f))
          continue;

        if (has_obtuse_angle(f)) {
          
          InsertionMethod steiner_method = choose_random_steiner_method();
          if (steiner_method == InsertionMethod::PROJECTION || 
              steiner_method == InsertionMethod::MIDPOINT || 
              steiner_method == InsertionMethod::CENTROID) {

            SteinerMethodObtPoint method;
            if (steiner_method == InsertionMethod::PROJECTION) method = insert_projection;
            else if (steiner_method == InsertionMethod::MIDPOINT) method = insert_mid;
            else if (steiner_method == InsertionMethod::CENTROID) method = insert_centroid;

            CDT copy(cdt);
            obt_point calc_insert_proj = method(copy, f);
            steiner_counter++;
            new_en = a * calc_insert_proj.obt_count + b * steiner_counter;
            de = new_en - cur_en;
            if (de < 0) {
              obt_point best_steiner = method(cdt, f);
              cdt.insert_steiner_x_y(best_steiner.insrt_pt.x(), best_steiner.insrt_pt.y());
              cur_en = new_en;
              flag = true;
              break;
            }
            else {
              double exponent = (-1*de) / T;
              double prob = std::pow(e, exponent);
              bool acc = accept_or_decline(prob);
              if (acc) {
                obt_point best_steiner = method(cdt, f);
                cdt.insert_steiner_x_y(best_steiner.insrt_pt.x(), best_steiner.insrt_pt.y());
                cur_en = new_en;
                flag = true;
                break;
              }
              else {
                steiner_counter--;
              }
            }
          }
          else if (steiner_method == InsertionMethod::CIRCUMCENTER ||
                    steiner_method == InsertionMethod::MERGE_OBTUSE) {

            SteinerMethodObtFace method;
            if (steiner_method == InsertionMethod::CIRCUMCENTER) method = insert_circumcenter;
            else if (steiner_method == InsertionMethod::MERGE_OBTUSE) method = merge_obtuse;

            CDT copy(cdt);
            FaceData face_data(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
            obt_face temp = method(copy, face_data);
            if (temp.obt_count == 9999) continue;
            steiner_counter++;
            new_en = a * temp.obt_count + b * steiner_counter;
            de = new_en - cur_en;
            if (de < 0) {
              FaceData face_data(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
              method(cdt, face_data);
              cur_en = new_en;
              flag = true;
              break;
            }
            else {
              double exponent = (-1*de) / T;
              double prob = std::pow(e, exponent);
              bool acc = accept_or_decline(prob);
              if (acc) {
                FaceData face_data(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
                method(cdt, face_data);
                cur_en = new_en;
                flag = true;
                break;
              }
              else {
                steiner_counter--;
              }
            }
          }
        }
      }

      if (count_obtuse_triangles(cdt) == 0) {
        std::cout << "current obtuse triangles: 0\n" << std::endl;
        return;
      }

      T = T - (double)((double)1 / (double)L);
      std::cout << "After sa try " << tries << " -> obt_triangles: " << count_obtuse_triangles(cdt) << " | steiner_counter: " << steiner_counter << "| T: " << T << std::endl;
    }
  }
}


// The Local Search algorithm
// Also the method used in the first assignment
void local_search(CDT& cdt, int L) {
  int i;
  for (i = 0 ; i < L ; i++) {

    if (count_obtuse_triangles(cdt) == 0) {
      break;
    }

    int init_obtuse_count = count_obtuse_triangles(cdt);
    Point starting_point;
    CDT::Face_handle starting_face;
    obt_point best_steiner(9999, starting_point);
    obt_face merge_face(9999, starting_face);
    obt_face circumcenter_face(9999, starting_face);
    InsertionMethod best_method = InsertionMethod::NONE;
    FaceData toReplaceFace(Point(0,0), Point(0,0), Point(0,0));

    auto start = std::chrono::high_resolution_clock::now();
    
    // Iterate the faces of the cdt
    for (CDT::Finite_faces_iterator f = cdt.finite_faces_begin(); f != cdt.finite_faces_end(); f++) {
      
      if (!is_triangle_inside_region_boundary(f))
        continue;

      if (has_obtuse_angle(f)) {
        
        CDT copy(cdt);
        obt_point calc_insert_proj = insert_projection(copy, f);
        if (best_steiner.obt_count > calc_insert_proj.obt_count) {
          best_steiner = calc_insert_proj;
          best_method = InsertionMethod::PROJECTION;
        }

        CDT copy1(cdt);
        obt_point calc_insert_mid = insert_mid(copy1, f);
        if (best_steiner.obt_count > calc_insert_mid.obt_count) {
          best_steiner = calc_insert_mid;
          best_method = InsertionMethod::MIDPOINT;
        }

        CDT copy2(cdt);
        obt_point calc_insert_centr = insert_centroid(copy2, f);
        if (best_steiner.obt_count > calc_insert_centr.obt_count) {
          best_steiner = calc_insert_centr;
          best_method = InsertionMethod::CENTROID;
        }

        CDT copy3(cdt);
        FaceData face_data(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
        obt_face temp_circ_face = insert_circumcenter(copy3, face_data);
        if (best_steiner.obt_count > temp_circ_face.obt_count) {
          circumcenter_face = temp_circ_face;
          toReplaceFace.p1 = f->vertex(0)->point();
          toReplaceFace.p2 = f->vertex(1)->point();
          toReplaceFace.p3 = f->vertex(2)->point();
          best_method = InsertionMethod::CIRCUMCENTER;
        }

        CDT copy4(cdt);
        FaceData face_data2(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
        obt_face temp_merge_face = merge_obtuse(copy4, face_data2);
        if (best_steiner.obt_count > temp_merge_face.obt_count) {
          merge_face = temp_merge_face;
          toReplaceFace.p1 = f->vertex(0)->point();
          toReplaceFace.p2 = f->vertex(1)->point();
          toReplaceFace.p3 = f->vertex(2)->point();
          best_method = InsertionMethod::MERGE_OBTUSE;
        }
      }
    }
    if (best_method == InsertionMethod::PROJECTION || 
        best_method == InsertionMethod::MIDPOINT || 
        best_method == InsertionMethod::CENTROID) {
      cdt.insert_no_flip(best_steiner.insrt_pt);
      cdt.insert_steiner_x_y(best_steiner.insrt_pt.x(), best_steiner.insrt_pt.y());
    }
    else if (best_method == InsertionMethod::CIRCUMCENTER) {
      insert_circumcenter(cdt, toReplaceFace);
    }
    else if (best_method == InsertionMethod::MERGE_OBTUSE) {
      merge_obtuse(cdt, toReplaceFace);
    }
    std::cout << "After local search try " << i << "-> obt_triangles: " << count_obtuse_triangles(cdt) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    if (elapsed.count() > 1) {
      std::cout << "Time limit reached\n";
      break;
    }
  }
  std::cout << "After " << i << " Steiner Insertions | obt_triangles: " << count_obtuse_triangles(cdt) << std::endl;
}

// Create the region_boundary_polygon polygon
Polygon_2 make_region_boundary_polygon(std::list<int> region_boundary, std::vector<Point> points) {
  Polygon_2 region_boundary_polygon;
  for (int temp : region_boundary) {
    region_boundary_polygon.push_back(points[temp]);
  }
  return region_boundary_polygon;
}

// Method to handle the algorithms and their parameters
void handle_methods(CDT& cdt, 
                    std::string method, 
                    std::list<std::pair<std::string, double>> parameters,
                    bool delaunay) {
  if (method == "local") {
    auto it = parameters.begin();
    double L = it->second;
    if (!delaunay) {
      local_search(cdt, 30);
      if (count_obtuse_triangles(cdt) == 0) {
        return;
      }
    }
    local_search(cdt, L);
  }
  else if (method == "sa") {
    auto it = parameters.begin();
    double alpha = it->second;
    it++;
    double beta = it->second;
    it++;
    double L = it->second;
    if (!delaunay) {
      local_search(cdt, 30);
      if (count_obtuse_triangles(cdt) == 0) {
        return;
      }
    }
    sim_annealing(cdt, alpha, beta, L);
  }
  else if (method == "ant") {
    auto it = parameters.begin();
    double alpha = it->second;
    it++;
    double beta = it->second;
    it++;
    double xi = it->second;
    it++;
    double psi = it->second;
    it++;
    double lambda = it->second;
    it++;
    double kappa = it->second;
    it++;
    double L = it->second;
    ant_parameters ant_params(alpha, beta, xi, psi, lambda, kappa, L);
    if (!delaunay) {
      local_search(cdt, 30);
      if (count_obtuse_triangles(cdt) == 0) {
        return;
      }
    }
    ant_colony_optimization(cdt, ant_params);
  }
}

using namespace read_write_file;

enum class InputCategory {
  CONVEX_NO_CONSTR,
  CONVEX_OPEN_CONSTR,
  CONVEX_CLOSED_CONSTR,
  NOT_CONVEX_PARALLEL,
  NOT_CONVEX_NO_RULES
};

bool vertex_touches_boundary(CDT::Vertex_handle v) {
  for (auto it = region_boundary_polygon.vertices_begin(); it != region_boundary_polygon.vertices_end(); ++it) {
    Point point = *it;
    if (equal_points(v->point(), point)) {
      return true;
    }
  }
  return false;
}

bool vertex_connects_constrained_edges(CDT& cdt, CDT::Vertex_handle v) {
  int counter = 0;
  auto it = cdt.incident_edges(v);
  auto start = it;
  do {
      if (cdt.is_constrained(*it)) {
          counter++;
      }
      it++;
  } while (it != start);
  if (counter >= 2) {
    return true;
  }
  return false;
}

CDT::Vertex_handle get_opposite_vertex(CDT& cdt, CDT::Vertex_handle v, Edge& e) {
  CDT::Vertex_handle vertex1 = get_vertex_from_edge(e, 1);
  CDT::Vertex_handle vertex2 = get_vertex_from_edge(e, 2);
  if (equal_points(v->point(), vertex1->point())) {
    return vertex2;
  }
  else {
    return vertex1;
  }
}

class VertexTouchingBoundary {
  public:
    std::list<CDT::Vertex_handle> vertices_touching;
    int count_vertex_touching_boundary;
    VertexTouchingBoundary(std::list<CDT::Vertex_handle> vertices_touching, int count_vertex_touching_boundary) : 
      vertices_touching(vertices_touching), count_vertex_touching_boundary(count_vertex_touching_boundary) {}
};

bool vertex_will_touch_boundary(CDT& cdt, CDT::Vertex_handle v, Edge e, VertexTouchingBoundary& vtb) {
  // if vertex touches boundary (and not already recorded it) return true
  std::cout << "vertex_will_touch_boundary\n";
  if (vertex_touches_boundary(v)) {
    for (CDT::Vertex_handle vertex : vtb.vertices_touching) {
      if (equal_points(vertex->point(), v->point())) {
        return false;
      }
    }
    vtb.count_vertex_touching_boundary++;
    vtb.vertices_touching.push_back(v);
    return true;
  }

  // if vertex doesn't touch boundary, check if it connects constrained edges
  // if it connects, for each edge it connects (except the one that originated from)
  // check if the opposite vertex will touch the boundary
  // if it does, return true
  // if it doesn't, repeat the process for the opposite vertex 
  
  if (vertex_connects_constrained_edges(cdt, v)) {
    for (auto it = cdt.incident_edges(v); it != nullptr; ++it) {
      if (cdt.is_constrained(*it)) {
        Edge edge = *it;
        if (!equal_edges(get_point_from_edge(e, 1), 
                        get_point_from_edge(e, 2),
                        get_point_from_edge(edge, 1), 
                        get_point_from_edge(edge, 2))) {
          CDT::Vertex_handle opposite_vertex = get_opposite_vertex(cdt, v, edge);
          if (vertex_will_touch_boundary(cdt, opposite_vertex, edge, vtb)) {
            return true;
          }
        }
      }
    }
  }


  return false;
}

bool edge_not_on_boundary(Edge e) {
  CDT::Vertex_handle vertex1 = get_vertex_from_edge(e, 1);
  CDT::Vertex_handle vertex2 = get_vertex_from_edge(e, 2);
  if (vertex_touches_boundary(vertex1) && vertex_touches_boundary(vertex2)) {
    return false;
  }
  return true;
}

// If at least two vertex of the same "path" of constrained edges touch the boundary, return true
bool has_closed_constraints(CDT& cdt) {
  int count_vertex_touching_boundary = 0;
  VertexTouchingBoundary vtb(std::list<CDT::Vertex_handle>(), 0);
  for (const Edge& e : cdt.finite_edges()) {
    if (cdt.is_constrained(e) && edge_not_on_boundary(e)) {
      CDT::Vertex_handle vertex1 = get_vertex_from_edge(e, 1);
      CDT::Vertex_handle vertex2 = get_vertex_from_edge(e, 2);
      vertex_will_touch_boundary(cdt, vertex1, e, vtb);
      vertex_will_touch_boundary(cdt, vertex2, e, vtb);
      if (vtb.count_vertex_touching_boundary >= 2) {
        return true;
      }
    }
  }
  return false;
}

bool has_parallel_edges(CDT& cdt) {

  // implement the parallel edges check
  return false;
}

InputCategory choose_input_category(CDT& cdt, std::list<std::pair<int, int>>& additional_constraints) {
  if (region_boundary_polygon.is_convex() && additional_constraints.empty()) {
    return InputCategory::CONVEX_NO_CONSTR;
  }
  else if (region_boundary_polygon.is_convex() && !has_closed_constraints(cdt)) {
    return InputCategory::CONVEX_OPEN_CONSTR;
  }
  else if (region_boundary_polygon.is_convex()) {
    return InputCategory::CONVEX_CLOSED_CONSTR;
  }
  else if (!region_boundary_polygon.is_convex() && has_parallel_edges(cdt)) {
    return InputCategory::NOT_CONVEX_PARALLEL;
  }
  else {
    return InputCategory::NOT_CONVEX_NO_RULES;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 5 || argc > 6) {
    std::cout << "Wrong number of arguments\n";
    return 1;
  }
  
  namespace pt = boost::property_tree; // namespace alias
  pt::ptree root; // create a root node

  // Choose the file to read
  pt::read_json(argv[2], root); // read the json file
  
  // Read the json file
  std::string instance_uid = get_instance_uid(root);
  int num_points = get_num_points(root);
  std::list<int> points_x = get_points_x(root);
  std::list<int> points_y = get_points_y(root);
  std::list<int> region_boundary = get_region_boundary(root);
  std::string num_constraints = get_num_constraints(root);
  std::list<std::pair<int, int>> additional_constraints = get_additional_constraints(root);
  std::list<std::pair<int, int>> constraints = combine_constraints(region_boundary, additional_constraints);
  std::string method;
  std::list<std::pair<std::string, double>> parameters;
  boost::property_tree::ptree parameters_for_output;
  bool delaunay;
  if (argc == 5) {
    method = get_method(root);
    parameters = get_parameters(root);
    parameters_for_output = root.get_child("parameters");
    delaunay = get_delaunay(root);
  }
  else {
    method = "local";
    parameters = {{"L", 90}};
    parameters_for_output.put("L", 90);
    delaunay = true;
  }

  // Create the Constrained Delaunay Triangulation (CDT)
  CDT cdt;

  // Define the points from the PSLG (x, y coordinates) and insert them into the CDT
  std::vector<Point> points = {};
  auto it_y = points_y.begin();
  for (const auto& p : points_x) {
    int point_x = p;
    int point_y = *it_y;
    points.push_back(Point(point_x, point_y));
    cdt.insert(Point(point_x, point_y));
    it_y++;
  }

  // Add the constrained edges from constraints
  for (const auto &constraint : constraints) {
    cdt.insert_constraint(points[constraint.first], points[constraint.second]);
  }

  // Create the region boundary polygon
  region_boundary_polygon = make_region_boundary_polygon(region_boundary, points);

  // Find input category
  InputCategory input_category = choose_input_category(cdt, additional_constraints);
  std::cout << "Input category: " << (int)input_category << std::endl;
  return 0; //TODO: delete later

  // Count the obtuse triangles
  std::cout << "Starting obtuse counter: " << count_obtuse_triangles(cdt) << std::endl;

  // Insert Steiner points
  CGAL::draw(cdt);
  handle_methods(cdt, method, parameters, delaunay);

  // Output the results
  write_output(cdt, points, method, parameters_for_output, argv[4]);
  CGAL::draw(cdt);
  
  return 0;
}