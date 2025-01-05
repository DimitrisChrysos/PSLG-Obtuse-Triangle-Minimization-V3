#include "input_categories.hpp"

// If the vertex is part of the boundary, return true
bool input_categories::vertex_touches_boundary(CDT::Vertex_handle v) {
  for (auto it = region_boundary_polygon.vertices_begin(); it != region_boundary_polygon.vertices_end(); ++it) {
    Point point = *it;
    if (equal_points(v->point(), point)) {
      return true;
    }
  }
  return false;
}

// If the vertex connects two (or more) constrained edges, return true
bool input_categories::vertex_connects_constrained_edges(CDT& cdt, CDT::Vertex_handle v) {
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

// Get the opposite vertex of an edge
CDT::Vertex_handle input_categories::get_opposite_vertex(CDT& cdt, CDT::Vertex_handle v, Edge& e) {
  CDT::Vertex_handle vertex1 = get_vertex_from_edge(e, 1);
  CDT::Vertex_handle vertex2 = get_vertex_from_edge(e, 2);
  if (equal_points(v->point(), vertex1->point())) {
    return vertex2;
  }
  else {
    return vertex1;
  }
}

// Explore the vertex and check if itself or another vertex 
// of a connected constrained edge touches the boundary
void input_categories::explore_vertex(CDT& cdt, CDT::Vertex_handle v, Edge e, VertexTouchingBoundary& vtb) {

  // if vertex has already been explored, return
  for (CDT::Vertex_handle vertex : vtb.vertices_explored) {
    if (equal_points(vertex->point(), v->point())) {
      return;
    }
  }

  // Add the vertex to the list of explored vertices and check if it touches the boundary
  vtb.vertices_explored.push_back(v);
  if (vertex_touches_boundary(v)) {
    vtb.count_vertex_touching_boundary++;
  }

  // If the vertex connects two constrained edges, explore the opposite vertex
  if (vertex_connects_constrained_edges(cdt, v)) {
    auto it = cdt.incident_edges(v);
    auto start = it;
    do {
        if (cdt.is_constrained(*it)) {
          Edge edge = *it;
          if (!equal_edges(get_point_from_edge(e, 1), 
                          get_point_from_edge(e, 2),
                          get_point_from_edge(edge, 1), 
                          get_point_from_edge(edge, 2))) {
            CDT::Vertex_handle opposite_vertex = get_opposite_vertex(cdt, v, edge);
            if (!(vertex_touches_boundary(v) && vertex_touches_boundary(opposite_vertex))) {
              explore_vertex(cdt, opposite_vertex, edge, vtb);
            }
          }
        }
        it++;
    } while (it != start);
  }
}

// If both vertices of a constrained edge touch the boundary, return true
bool input_categories::edge_on_boundary(Edge e) {
  CDT::Vertex_handle vertex1 = get_vertex_from_edge(e, 1);
  CDT::Vertex_handle vertex2 = get_vertex_from_edge(e, 2);
  if (vertex_touches_boundary(vertex1) && vertex_touches_boundary(vertex2)) {
    return true;
  }
  return false;
}

// If at least two vertex of the same "path" of constrained edges touch the boundary, return true
bool input_categories::has_closed_constraints(CDT& cdt) {

  VertexTouchingBoundary vtb(std::list<CDT::Vertex_handle>(), 0); // vtb -> vertex touching boundary
  for (const Edge& e : cdt.finite_edges()) {
    if (cdt.is_constrained(e) && !edge_on_boundary(e)) {
      CDT::Vertex_handle vertex1 = get_vertex_from_edge(e, 1);
      CDT::Vertex_handle vertex2 = get_vertex_from_edge(e, 2);
      explore_vertex(cdt, vertex1, e, vtb);
      explore_vertex(cdt, vertex2, e, vtb);
      if (vtb.count_vertex_touching_boundary >= 2) {
        return true;
      }
    }
  }
  return false;
}

// If all the edges of the region boundary polygon are parallel (and it has no additional constraints), return true
bool input_categories::has_parallel_edges(CDT& cdt) {
  for (const Edge& e : cdt.finite_edges()) {
    if (!cdt.is_constrained(e)) continue;
    Point point1 = get_point_from_edge(e, 1);
    Point point2 = get_point_from_edge(e, 2);
    double p1x = CGAL::to_double(point1.x());
    double p1y = CGAL::to_double(point1.y());
    double p2x = CGAL::to_double(point2.x());
    double p2y = CGAL::to_double(point2.y());
    if ((p1x - p2x != 0) && (p1y - p2y != 0)) {
      return false;
    }
  }
  return true;
}

// Find the input category of the CDT
input_categories::InputCategory input_categories::find_input_category(CDT& cdt, std::list<std::pair<int, int>>& additional_constraints) {
  if (region_boundary_polygon.is_convex() && additional_constraints.empty()) {
    return input_categories::InputCategory::CONVEX_NO_CONSTR;
  }
  else if (region_boundary_polygon.is_convex() && !has_closed_constraints(cdt)) {
    return input_categories::InputCategory::CONVEX_OPEN_CONSTR;
  }
  else if (region_boundary_polygon.is_convex()) {
    return input_categories::InputCategory::CONVEX_CLOSED_CONSTR;
  }
  else if (!region_boundary_polygon.is_convex() && additional_constraints.empty() && has_parallel_edges(cdt)) {
    return input_categories::InputCategory::NOT_CONVEX_PARALLEL;
  }
  else {
    return input_categories::InputCategory::NOT_CONVEX_NO_RULES;
  }
}