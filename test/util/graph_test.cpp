/*
 * graph_test.cpp
 *
 *  Created on: Dec 2, 2017
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>

#include <wire/util/graph.hpp>
#include <vector>

namespace wire {
namespace util {
namespace graph {
namespace test {

using graph_type = ::std::map< char, ::std::vector< char > >;
struct output_visitor : visitor_base {
    void
    discover_vertex(char c)
    {
        ::std::cout << "Discover vertex " << c << "\n";
        discover.push_back(c);
    }
    void
    examine_vertex(char c)
    {
        ::std::cout << "Examine vertex " << c << "\n";
    }

    void
    finish_vertex(char c)
    {
        ::std::cout << "*** Finish vertex " << c << "\n";
        finish.push_back(c);
    }
    void
    forward_edge(char s, char t)
    {
        ::std::cout << "Forward edge " << s << "->" << t << "\n";
    }
    void
    back_edge(char s, char t)
    {
        ::std::cout << "Back edge " << s << "->" << t << "\n";
    }

    ::std::string discover;
    ::std::string finish;
};

graph_type graph {
    { 'a', { 'b', 'c', 'e' }},
    { 'b', { 'd', 'f' }},
    { 'c', { 'g' } },
    { 'd', {} },
    { 'e', {} },
    { 'f', {} },
    { 'g', { 'e' } },
};

auto
get_adjacent(char c)
{
    return ::std::make_pair(graph[c].begin(), graph[c].end());
}

TEST(Util, DFSVisit)
{
    using traits_type = vertex_traits<char>;

    traits_type::color_map colors;
    output_visitor vis;
    dfs_vertex_visit('a', colors, vis, get_adjacent);

    EXPECT_EQ("abdfcge", vis.discover);
    EXPECT_EQ("dfbegca", vis.finish);
    ::std::cout << vis.discover << "\n";
    ::std::cout << vis.finish << "\n";
}

TEST(Util, BFSVisit)
{
    using traits_type = vertex_traits<char>;

    traits_type::color_map colors;
    output_visitor vis;
    bfs_vertex_visit('a', colors, vis, get_adjacent);

    ::std::cout << vis.discover << "\n";
    ::std::cout << vis.finish << "\n";
}

}  /* namespace test */
}  /* namespace graph */
}  /* namespace util */
}  /* namespace wire */
