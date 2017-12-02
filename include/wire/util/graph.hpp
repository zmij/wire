/*
 * graph.hpp
 *
 *  Created on: Dec 2, 2017
 *      Author: sergey.fedorov
 */

#ifndef WIRE_UTIL_GRAPH_HPP_
#define WIRE_UTIL_GRAPH_HPP_

#include <type_traits>
#include <memory>
#include <map>
#include <stack>
#include <queue>
#include <exception>

namespace wire {
namespace util {
namespace graph {

enum class vertex_colors {
    white   = 0,
    gray,
    black
};

namespace detail {

template < typename T >
struct default_vertex_identity {
    using vertex_type   = T;
    using identity_type = T const*;
    using pointer       = T const*;
    using reference     = T const&;

    static inline identity_type
    get_id(vertex_type const& v)
    { return &v; }

    static inline pointer
    get_pointer(vertex_type const& v)
    { return &v; }

    static inline reference
    get_reference(pointer p)
    { return *p; }
};

template < typename T >
struct fundamental_vertex_identity {
    using vertex_type   = T;
    using identity_type = T;
    using pointer       = T const*;
    using reference     = T const&;

    static inline identity_type
    get_id(vertex_type const& v)
    { return v; }

    static inline pointer
    get_pointer(vertex_type const& v)
    { return &v; }

    static inline reference
    get_reference(pointer p)
    { return *p; }
};

}  /* namespace detail */

template < typename T >
struct vertex_identity :
    ::std::conditional< ::std::is_fundamental<T>::value,
         detail::fundamental_vertex_identity<T>,
         detail::default_vertex_identity<T> >::type {
};

template < typename T >
struct vertex_identity< T* > {
    using vertex_type   = T*;
    using identity_type = T*;
    using pointer       = T*;
    using reference     = T*;

    static inline identity_type
    get_id(vertex_type v)
    { return v; }

    static inline pointer
    get_pointer(vertex_type const& v)
    { return v; }

    static inline reference
    get_reference(pointer p)
    { return p; }
};

template < typename T >
struct vertex_identity< ::std::shared_ptr<T> > {
    using vertex_type   = ::std::shared_ptr<T>;
    using identity_type = ::std::shared_ptr<T>;
    using pointer       = ::std::shared_ptr<T>;
    using reference     = ::std::shared_ptr<T>;

    static inline identity_type
    get_id(vertex_type const& v)
    { return v; }

    static inline pointer
    get_pointer(vertex_type const& v)
    { return v; }

    static inline reference
    get_reference(pointer p)
    { return p; }
};

template < typename T >
struct vertex_traits {
    using identity      = vertex_identity< T >;
    using vertex_type   = typename identity::vertex_type;
    using identity_type = typename identity::identity_type;

    using color_map     = ::std::map< identity_type, vertex_colors >;
};

/**
 * Base structure for visiting vertices and/or edges in different algorithms
 */
struct visitor_base {
    template< typename VertexType >
    void
    discover_vertex(VertexType const& vertex) {}

    template< typename VertexType >
    void
    examine_vertex(VertexType const& vertex) {}

    template< typename EdgeType >
    void
    forward_edge(EdgeType const&) {}

    template< typename VertexType >
    void
    forward_edge(VertexType const& from, VertexType const& to) {}

    template< typename VertexType, typename EdgeType >
    void
    forward_edge(VertexType const& from, VertexType const& to,
            EdgeType const& edge) {}

    template< typename EdgeType >
    void
    back_edge(EdgeType const&) {}

    template< typename VertexType >
    void
    back_edge(VertexType const&, VertexType const&) {}

    template< typename VertexType, typename EdgeType >
    void
    back_edge(VertexType const& from, VertexType const& to,
            EdgeType const& edge) {}

    template< typename EdgeType >
    void
    forward_or_cross_edge(EdgeType const&) {}

    template< typename VertexType >
    void
    forward_or_cross_edge(VertexType const& from, VertexType const& to) {}

    template< typename VertexType, typename EdgeType >
    void
    forward_or_cross_edge(VertexType const& from, VertexType const& to,
            EdgeType const& edge) {}

    template< typename EdgeType >
    bool
    proceed_on_edge(EdgeType const&)
    { return true; }

    template< typename VertexType >
    void
    finish_vertex(VertexType const& vertex) {}
};

/** Depth-first visit a node of graph.
 * Algorithm for depth-first graph traversal.
 * @tparam VertexType Type of vertex.
 * @tparam Visitor Type of visitor to notify about DFS events.
 * @tparam AdjuscentEdgesFunctor Functor type returning adjacent vertices range
 *                                 (a pair of iterators).
 * @param vertex Vertex to visit.
 * @param colors Map of colors to mark vertices unvisited/processing/visited
 * @param vis Visitor object
 * @param get_adjascent Functor returning a pair of iterators to traverse
 *                      adjacent vertices.
 */
template< typename VertexType, typename Visitor,
        typename AdjacentVerticesFunctor >
void
dfs_vertex_visit(VertexType const& vertex,
        typename vertex_traits< VertexType >::color_map& colors,
        Visitor& vis, AdjacentVerticesFunctor get_adjacent)
{
    /** @todo Check algo concepts */
    using traits_type = vertex_traits< VertexType >;
    using identity    = typename traits_type::identity;
    using vertex_stack  = ::std::stack< typename identity::pointer >;

    vertex_stack stack;
    auto id = identity::get_id(vertex);
    colors[id] = vertex_colors::gray;
    // discover vertex
    vis.discover_vertex(vertex);

    auto adjacent = get_adjacent(vertex);
    for (auto p = adjacent.first; p != adjacent.second; ++p) {
        auto pid = identity::get_id(*p);
        if (colors[pid] == vertex_colors::white) {
            vis.forward_edge(vertex, *p);
            dfs_vertex_visit(*p, colors, vis, get_adjacent);
        } else if (colors[pid] == vertex_colors::gray) {
            vis.back_edge(vertex, *p);
        } else if (colors[pid] == vertex_colors::black) {
            vis.forward_or_cross_edge(vertex, *p);
        }
    }
    colors[id] = vertex_colors::black;
    // finish vertex
    vis.finish_vertex(vertex);
}

/** Depth-first visit a range of nodes in a graph.
 * Algorithm for depth-first graph traversal.
 * @tparam VertexIterator Iterator type over a range of vertices.
 * @tparam Visitor Type of visitor to notify about DFS events.
 * @tparam AdjacentVerticesFunctor Functor type returning adjacent vertices range
 *                                 (a pair of iterators).
 * @param first Iterator to first vertex in range.
 * @param last Iterator after the last vertex in range.
 * @param colors Map of colors to mark vertices unvisited/processing/visited
 * @param vis Visitor object
 * @param get_adjacent Functor returning a pair of iterators to traverse
 *                      adjacent vertices.
 */
template< typename VertexIterator, typename Visitor,
        typename AdjacentVerticesFunctor >
void
dfs_vertex_visit(
        VertexIterator first,
        VertexIterator last,
        typename vertex_traits< typename VertexIterator::value_type >::color_map& colors,
        Visitor& vis, AdjacentVerticesFunctor get_adjacent)
{
    using vertex_type   = typename VertexIterator::value_type;
    using traits_type   = vertex_traits< vertex_type >;
    using identity      = typename traits_type::identity;

    for (; first != last; ++first) {
        auto id = identity::get_id(*first);

        if (colors[id] == vertex_colors::white) {
            dfs_vertex_visit(*first, colors, vis, get_adjacent);
        }
    }
}

/** @todo Depth-first edges visit */

/** Breadth-first visit a node of graph.
 * Algorithm for breadth-first graph traversal.
 * @tparam VertexType Type of vertex.
 * @tparam Visitor Type of visitor to notify about DFS events.
 * @tparam AdjacentVerticesFunctor Functor type returning adjacent vertices range
 *                                 (a pair of iterators).
 * @param vertex Vertex to visit.
 * @param colors Map of colors to mark vertices unvisited/processing/visited
 * @param vis Visitor object
 * @param get_adjascent Functor returning a pair of iterators to traverse
 *                      adjacent vertices.
 */
template< typename VertexType, typename Visitor,
        typename AdjacentVerticesFunctor >
void
bfs_vertex_visit(VertexType const& vertex,
        typename vertex_traits< VertexType >::color_map& colors,
        Visitor& vis, AdjacentVerticesFunctor get_adjacent)
{
    /** @todo Check algo concepts */
    using traits_type   = vertex_traits< VertexType >;
    using identity      = typename traits_type::identity;
    using vertex_queue = ::std::queue< typename identity::pointer >;

    vertex_queue queue;
    auto id = identity::get_id(vertex);
    colors[id] = vertex_colors::gray;

    // discover vertex
    vis.discover_vertex(vertex);
    queue.push(identity::get_pointer(vertex));

    while (!queue.empty()) {
        auto pv = queue.front();
        queue.pop();
        auto const& v = identity::get_reference(pv);
        id = identity::get_id(v);
        // examine vertex
        vis.examine_vertex(v);
        auto adjacent = get_adjacent(v);
        for (auto p = adjacent.first; p != adjacent.second; ++p) {
            // examine edge
            auto pid = identity::get_id(*p);
            if (colors.find(pid) == colors.end())
                colors[pid] = vertex_colors::white;

            if (colors[pid] == vertex_colors::white) {
                // tree edge
                vis.forward_edge(v, *p);
                colors[pid] = vertex_colors::gray;
                // discover vertex
                vis.discover_vertex(*p);
                queue.push(identity::get_pointer(*p));
            } else if (colors[pid] == vertex_colors::gray) {
                // back edge
                vis.back_edge(v, *p);
            } else if (colors[pid] == vertex_colors::black) {
                // forward or cross edge
                vis.forward_or_cross_edge(v, *p);
            }
        } // for
        colors[id] = vertex_colors::black;
        vis.finish_vertex(v);
    } // while
}

/** Breadth-first visit a range of nodes in a graph.
 * Algorithm for breadth-first graph traversal.
 * @tparam VertexIterator Iterator type over a range of vertices.
 * @tparam Visitor Type of visitor to notify about DFS events.
 * @tparam AdjacentVerticesFunctor Functor type returning adjacent vertices range
 *                                 (a pair of iterators).
 * @param first Iterator to first vertex in range.
 * @param last Iterator after the last vertex in range.
 * @param colors Map of colors to mark vertices unvisited/processing/visited
 * @param vis Visitor object
 * @param get_adjascent Functor returning a pair of iterators to traverse
 *                      adjacent vertices.
 */
template< typename VertexIterator, typename Visitor,
        typename AdjacentVerticesFunctor >
void
bfs_vertex_visit(
        VertexIterator first,
        VertexIterator last,
        typename vertex_traits< typename VertexIterator::value_type >::color_map& colors,
        Visitor& vis, AdjacentVerticesFunctor get_adjascent)
{
    using vertex_type   = typename VertexIterator::value_type;
    using traits_type   = vertex_traits< vertex_type >;
    using identity      = typename traits_type::identity;

    for (; first != last; ++first) {
        auto id = identity::get_id(*first);

        if (colors[id] == vertex_colors::white) {
            bfs_vertex_visit(*first, colors, vis, get_adjascent);
        }
    }
}

/**
 * Visitor for sorting vertices topologically
 */
template < typename VertexType, typename OutputIterator >
struct topo_sort_visitor : visitor_base {
    using traits_type   = vertex_traits< VertexType >;
    using identity      = typename traits_type::identity;

    topo_sort_visitor(OutputIterator out)
        : out_{out} {}

    void
    back_edge(VertexType const& start, VertexType const& end)
    {
        throw ::std::runtime_error{ "Graph has cycles and cannot be topologically sorted" };
    }
    void
    finish_vertex(VertexType const& v)
    {
        *out_++ = identity::get_pointer(v);
    }
private:
    OutputIterator out_;
};

template < typename VertexIterator, typename OutputIterator,
        typename AdjacentVerticesFunctor >
void
topo_sort(VertexIterator first, VertexIterator last, OutputIterator out,
        AdjacentVerticesFunctor get_adjacent)
{
    using vertex_type   = typename VertexIterator::value_type;
    using traits_type   = vertex_traits< vertex_type >;
    using identity      = typename traits_type::identity;
    using color_map     = typename traits_type::color_map;

    color_map colors;
    topo_sort_visitor< vertex_type, OutputIterator > vis{out};

    dfs_vertex_visit(first, last, colors, vis, get_adjacent);
}

}  /* namespace graph */
}  /* namespace util */
}  /* namespace wire */



#endif /* WIRE_UTIL_GRAPH_HPP_ */
