#ifndef INPUT_CATEGORIES_HPP
#define INPUT_CATEGORIES_HPP

#include "../custom_cdt_class/custom_cdt_class.hpp"
#include "../utils/utils.hpp"

typedef CGAL::Constrained_triangulation_plus_2<custom_cdt_class::Custom_Constrained_Delaunay_triangulation_2<K, CGAL:: Default, Itag>> CDT;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef K::Segment_2 Segment;
typedef CDT::Point Point;
typedef CDT::Edge Edge;

using namespace utils;

namespace input_categories {
    
    enum class InputCategory {
        CONVEX_NO_CONSTR,
        CONVEX_OPEN_CONSTR,
        CONVEX_CLOSED_CONSTR,
        NOT_CONVEX_PARALLEL,
        NOT_CONVEX_NO_RULES
    };

    class VertexTouchingBoundary {
        public:
            std::list<CDT::Vertex_handle> vertices_explored;
            int count_vertex_touching_boundary;

            VertexTouchingBoundary(std::list<CDT::Vertex_handle> vertices_explored, int count_vertex_touching_boundary) : 
                vertices_explored(vertices_explored), count_vertex_touching_boundary(count_vertex_touching_boundary) {}
    };

    class ExploreCycles {
        public:
            std::list<CDT::Vertex_handle> vertices_explored;
            bool found_cycle;

            ExploreCycles(std::list<CDT::Vertex_handle> vertices_explored, bool found_cycle) : 
                vertices_explored(vertices_explored), found_cycle(found_cycle) {}
    };
    
    // If the vertex is part of the boundary, return true
    bool vertex_touches_boundary(CDT::Vertex_handle v);

    // If the vertex connects two (or more) constrained edges, return true
    bool vertex_connects_constrained_edges(CDT& cdt, CDT::Vertex_handle v);

    // Get the opposite vertex of an edge
    CDT::Vertex_handle get_opposite_vertex(CDT& cdt, CDT::Vertex_handle v, Edge& e);
    
    // Explore the vertex and check if itself or another vertex 
    // of a connected constrained edge touches the boundary
    void explore_vertex(CDT& cdt, CDT::Vertex_handle v, Edge e, VertexTouchingBoundary& vtb);

    // Explore if we can find a cycle of constrained edges, starting from v (exculing region boundary)
    void explore_cycles(CDT& cdt, CDT::Vertex_handle v, Edge e, ExploreCycles& ec);

    // If both vertices of a constrained edge touch the boundary, return true
    bool edge_on_boundary(Edge e);

    // If at least two vertex of the same "path" of constrained edges touch the boundary, return true
    bool has_closed_constraints(CDT& cdt);

    // If all the edges of the region boundary polygon are parallel (and it has no additional constraints), return true
    bool has_parallel_edges(CDT& cdt);

    // Find the input category of the CDT
    InputCategory find_input_category(CDT& cdt, std::list<std::pair<int, int>>& additional_constraints);
}

#endif // INPUT_CATEGORIES_HPP
