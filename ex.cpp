#include "includes/custom_cdt_class/custom_cdt_class.hpp"
#include "includes/utils/utils.hpp"
#include "includes/read_write_file/read_write_file.hpp"
#include "includes/steiner_methods/steiner_methods.hpp"
#include "includes/input_categories/input_categories.hpp"
#include "includes/evaluate_instance/evaluate_instance.hpp"
#include <random>
#include <chrono>
#include <cmath>
#include <fstream>


typedef CGAL::Constrained_triangulation_plus_2<custom_cdt_class::Custom_Constrained_Delaunay_triangulation_2<K, CGAL:: Default, Itag>> CDT;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef K::Segment_2 Segment;
typedef CDT::Point Point;
typedef CDT::Edge Edge;

using namespace utils;
using namespace steiner_methods;
using namespace input_categories;
using namespace read_write_file;
using namespace evaluate_instance;

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
InsertionMethod choose_steiner_method(CDT& cdt, CDT::Face_handle face, double k, double xi, double psi, t_sp tsp, AvailableSteinerMethods available_steiner_methods) {

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

  // Adjust probabilities directly based on availability
  double adjusted_p_projection = available_steiner_methods.proj ? p_projection : 0.0;
  double adjusted_p_circumcenter = available_steiner_methods.circum ? p_circumcenter : 0.0;
  double adjusted_p_midpoint = available_steiner_methods.mid ? p_midpoint : 0.0;
  double adjusted_p_merge_obtuse = available_steiner_methods.merge ? p_merge_obtuse : 0.0;

  // Compute sum of adjusted probabilities
  double sum_of_adjusted_probabilities = adjusted_p_projection + adjusted_p_circumcenter + adjusted_p_midpoint + adjusted_p_merge_obtuse;

  // Compute sum of probabilities and their previous
  double sum_p1 = adjusted_p_projection;
  double sum_p2 = sum_p1 + adjusted_p_circumcenter;
  double sum_p3 = sum_p2 + adjusted_p_midpoint;
  double sum_p4 = sum_p3 + adjusted_p_merge_obtuse;

  // Generate a random number between 0 and sum_of_adjusted_probabilities
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, sum_of_adjusted_probabilities);
  double random_number = dis(gen);

  // Select the method based on sum probabilities
  if (random_number <= sum_p1) {
    return InsertionMethod::PROJECTION;
  } else if (random_number <= sum_p2) {
    return InsertionMethod::CIRCUMCENTER;
  } else if (random_number <= sum_p3) {
    return InsertionMethod::MIDPOINT;
  } else if (random_number <= sum_p4) {
    return InsertionMethod::MERGE_OBTUSE;
  }

  // In case of an error, return the projection method
  return InsertionMethod::PROJECTION;
}

// Improve the triangulation
effective_ant improve_trianglulation(CDT& cdt, double k, ant_parameters ant_params, t_sp tsp, AvailableSteinerMethods available_steiner_methods) {

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
  InsertionMethod steiner_method;
  SteinerMethodObtPoint method_point;
  SteinerMethodObtFace method_face;
  bool found_method = false;
  while(!found_method) {
    steiner_method = choose_steiner_method(cdt, random_face, k, ant_params.xi, ant_params.psi, tsp, available_steiner_methods);
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
        if (!available_steiner_methods.centr) {
          continue;
        }
        steiner_method = InsertionMethod::CENTROID;
        method_point = insert_centroid;
      }
    }
    found_method = true;
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
void use_triangulation_ants(CDT& cdt, std::list<effective_ant>& ants, bool final_use = false) {
  for (effective_ant& ant : ants) {

    int save_obt_count = count_obtuse_triangles(cdt);
    int save_steiner_count = ants.size();
    
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
    if (final_use) {
      calculate_rate_of_convergence(cdt, save_obt_count, save_steiner_count);
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
void ant_colony_optimization(CDT& cdt, ant_parameters ant_params, AvailableSteinerMethods available_steiner_methods) {

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
      effective_ant new_ant = improve_trianglulation(copy, k, ant_params, tsp, available_steiner_methods);
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
  use_triangulation_ants(cdt, best_triangulation_ants, true);
  
  // For the random method
  cdt.value_pre_random = calculate_energy(count_obtuse_triangles(cdt), cdt.steiner_x.size());
  use_insert_random(cdt);

  std::cout << "\nFinal -> Obtuse Triangles: " << count_obtuse_triangles(cdt) << " || Steiner Points: " << cdt.steiner_x.size() << std::endl;
  }

// Accept or decline something with the given probability
bool accept_or_decline(double prob) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::bernoulli_distribution dist(prob);
  return dist(gen);
}

// The simulated annealing algorithm
void sim_annealing(CDT& cdt, double a, double b, int L, AvailableSteinerMethods available_steiner_methods) {
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
          
          InsertionMethod steiner_method = choose_random_steiner_method(available_steiner_methods);
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
              int save_obt_count = count_obtuse_triangles(cdt);
              int save_steiner_count = cdt.steiner_x.size();
              obt_point best_steiner = method(cdt, f);
              cdt.insert_steiner_x_y(best_steiner.insrt_pt.x(), best_steiner.insrt_pt.y());
              calculate_rate_of_convergence(cdt, save_obt_count, save_steiner_count);
              cur_en = new_en;
              flag = true;
              break;
            }
            else {
              double exponent = (-1*de) / T;
              double prob = std::pow(e, exponent);
              bool acc = accept_or_decline(prob);
              if (acc) {
                int save_obt_count = count_obtuse_triangles(cdt);
                int save_steiner_count = cdt.steiner_x.size();
                obt_point best_steiner = method(cdt, f);
                cdt.insert_steiner_x_y(best_steiner.insrt_pt.x(), best_steiner.insrt_pt.y());
                calculate_rate_of_convergence(cdt, save_obt_count, save_steiner_count);
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
              int save_obt_count = count_obtuse_triangles(cdt);
              int save_steiner_count = cdt.steiner_x.size();
              FaceData face_data(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
              method(cdt, face_data);
              calculate_rate_of_convergence(cdt, save_obt_count, save_steiner_count);
              cur_en = new_en;
              flag = true;
              break;
            }
            else {
              double exponent = (-1*de) / T;
              double prob = std::pow(e, exponent);
              bool acc = accept_or_decline(prob);
              if (acc) {
                int save_obt_count = count_obtuse_triangles(cdt);
                int save_steiner_count = cdt.steiner_x.size();
                FaceData face_data(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point());
                method(cdt, face_data);
                calculate_rate_of_convergence(cdt, save_obt_count, save_steiner_count);
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

  // For the random method
  cdt.value_pre_random = calculate_energy(count_obtuse_triangles(cdt), cdt.steiner_x.size());
  use_insert_random(cdt);
}


// The Local Search algorithm
// Also the method used in the first assignment
void local_search(CDT& cdt, int L, AvailableSteinerMethods available_steiner_methods) {
  int i;
  int consec_insertions = 0;
  for (i = 0 ; i < L ; i++) {
    
    int init_obtuse_count = count_obtuse_triangles(cdt);
    if (init_obtuse_count == 0) {
      break;
    }

    if (consec_insertions >= 30)
      break;

    // int init_obtuse_count = count_obtuse_triangles(cdt);
    Point starting_point;
    CDT::Face_handle starting_face;
    obt_point best_steiner(9999, starting_point);
    obt_face merge_face(9999, starting_face);
    obt_face circumcenter_face(9999, starting_face);
    InsertionMethod best_method = InsertionMethod::NONE;
    FaceData toReplaceFace(Point(0,0), Point(0,0), Point(0,0));

    int save_obt_count = count_obtuse_triangles(cdt);
    int save_steiner_count = cdt.steiner_x.size();

    auto start = std::chrono::high_resolution_clock::now();
    
    // Iterate the faces of the cdt
    for (CDT::Finite_faces_iterator f = cdt.finite_faces_begin(); f != cdt.finite_faces_end(); f++) {
      
      if (!is_triangle_inside_region_boundary(f))
        continue;

      if (has_obtuse_angle(f)) {
        
        if (available_steiner_methods.proj) {
          CDT copy(cdt);
          obt_point calc_insert_proj = insert_projection(copy, f);
          if (best_steiner.obt_count > calc_insert_proj.obt_count) {
            best_steiner = calc_insert_proj;
            best_method = InsertionMethod::PROJECTION;
          }
        }

        if (available_steiner_methods.mid) {
          CDT copy1(cdt);
          obt_point calc_insert_mid = insert_mid(copy1, f);
          if (best_steiner.obt_count > calc_insert_mid.obt_count) {
            best_steiner = calc_insert_mid;
            best_method = InsertionMethod::MIDPOINT;
          }
        }

        if (available_steiner_methods.centr) {
          CDT copy2(cdt);
          obt_point calc_insert_centr = insert_centroid(copy2, f);
          if (best_steiner.obt_count > calc_insert_centr.obt_count) {
            best_steiner = calc_insert_centr;
            best_method = InsertionMethod::CENTROID;
          }
        }

        if (available_steiner_methods.circum) {
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
        }

        if (available_steiner_methods.merge) {
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
    calculate_rate_of_convergence(cdt, save_obt_count, save_steiner_count);
    std::cout << "After local search try " << i << "-> obt_triangles: " << count_obtuse_triangles(cdt) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    if (elapsed.count() > 1) {
      std::cout << "Time limit reached\n";
      break;
    }

    if (count_obtuse_triangles(cdt) >= init_obtuse_count) {
      consec_insertions++;
    }
    else {
      consec_insertions = 0;
    }
  }

  // For the random method
  if (i < L) {
    cdt.value_pre_random = calculate_energy(count_obtuse_triangles(cdt), cdt.steiner_x.size());
    use_insert_random(cdt);
  }

  std::cout << "After " << cdt.steiner_x.size() << " Steiner Insertions | obt_triangles: " << count_obtuse_triangles(cdt) << std::endl;
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
                    bool delaunay,
                    AvailableSteinerMethods available_steiner_methods) {
  if (method == "local") {
    auto it = parameters.begin();
    double L = it->second;
    if (!delaunay) {
      local_search(cdt, 30, available_steiner_methods);
      if (count_obtuse_triangles(cdt) == 0) {
        return;
      }
    }
    local_search(cdt, L, available_steiner_methods);
  }
  else if (method == "sa") {
    auto it = parameters.begin();
    double alpha = it->second;
    it++;
    double beta = it->second;
    it++;
    double L = it->second;
    if (!delaunay) {
      local_search(cdt, 30, available_steiner_methods);
      if (count_obtuse_triangles(cdt) == 0) {
        return;
      }
    }
    sim_annealing(cdt, alpha, beta, L, available_steiner_methods);
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
      local_search(cdt, 30, available_steiner_methods);
      if (count_obtuse_triangles(cdt) == 0) {
        return;
      }
    }
    ant_colony_optimization(cdt, ant_params, available_steiner_methods);
  }
}

// Main 
int main(int argc, char *argv[]) {
  if (argc < 4) {
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
  InputCategory input_category = find_input_category(cdt, additional_constraints);
  std::cout << "Input category: " << (int)input_category << std::endl;

  // Scan Config
  std::string method;
  std::list<std::pair<std::string, double>> parameters;
  boost::property_tree::ptree parameters_for_output;
  bool delaunay;
  AvailableSteinerMethods available_steiner_methods = {false, false, false, false, false};
  scan_config(argc, argv, root, method, parameters, parameters_for_output, delaunay, available_steiner_methods, input_category);


  // Used for the count_categories.py file
  if (argc == 4) {
    return 0;
  }

  // Count the obtuse triangles
  std::cout << "Starting obtuse counter: " << count_obtuse_triangles(cdt) << std::endl;

  // Insert Steiner points
  // CGAL::draw(cdt);
  handle_methods(cdt, method, parameters, delaunay, available_steiner_methods);

  // Function for the case that the program was called from the test_instances.py file
  for_test_instances_dot_py(argc, argv, cdt);

  // Output the results
  write_output(cdt, points, method, parameters_for_output, argv[4]);
  // CGAL::draw(cdt);
  
  return 0;
}