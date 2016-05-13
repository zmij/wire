/*
 * cpp_stream.hpp
 *
 *  Created on: May 13, 2016
 *      Author: zmij
 */

#ifndef WIRE_WIRE2CPP_CPP_SOURCE_STREAM_HPP_
#define WIRE_WIRE2CPP_CPP_SOURCE_STREAM_HPP_

#include <wire/idl/ast.hpp>
#include <fstream>
#include <boost/filesystem.hpp>
#include <deque>

namespace wire {
namespace idl {
namespace cpp {

class source_stream {
public:
    using path          = ::boost::filesystem::path;
    using string_list   = ::std::vector< ::std::string >;
public:
    source_stream(path const& origin_path, path const& header_dir,
            string_list const& include_dirs);
    ~source_stream();

    void
    open(path const&);

    source_stream&
    include(::std::string const& include_str);

    source_stream&
    include(char const* s)
    {
        return include(::std::string{s});
    }

    source_stream&
    include(path path);

    source_stream&
    include(::std::initializer_list<::std::string> const& includes)
    {
        for (auto const& inc : includes) {
            include(inc);
        }
        return *this;
    }


    template < template <typename, typename ...> class Container, typename ... Rest >
    source_stream&
    include(Container< ::std::string, Rest ... > const& includes)
    {
        for (auto const& inc : includes) {
            include(inc);
        }
        return *this;
    }
private:
    struct scope {
        ::std::string   name;
        bool            is_namespace;
    };
    using scope_stack   = ::std::deque<scope>;
private:
    template < typename T >
    friend source_stream&
    operator << (source_stream& os, T const& v);

    path            origin_path_;
    path            header_dir_;
    string_list     include_dirs_;

    ::std::ofstream stream_;

    bool            is_header_;
    ::std::string   header_guard_;

    scope_stack     current_scope_;
};

template < typename T >
source_stream&
operator << (source_stream& os, T const& v)
{
    os.stream_ << v;
    return os;
}

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_WIRE2CPP_CPP_SOURCE_STREAM_HPP_ */
