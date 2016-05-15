/*
 * cpp_stream.hpp
 *
 *  Created on: May 13, 2016
 *      Author: zmij
 */

#ifndef WIRE_WIRE2CPP_CPP_SOURCE_STREAM_HPP_
#define WIRE_WIRE2CPP_CPP_SOURCE_STREAM_HPP_

#include <wire/idl/ast.hpp>
#include <wire/wire2cpp/mapped_type.hpp>

#include <boost/filesystem.hpp>

#include <fstream>
#include <sstream>
#include <deque>

namespace wire {
namespace idl {
namespace cpp {

namespace annotations {

::std::string const CPP_CONTAINER = "cpp_container";
::std::string const GENERATE_CMP = "cpp_cmp";
::std::string const GENERATE_IO = "cpp_io";

}  /* namespace annotations */

class source_stream {
public:
    using path                  = ::boost::filesystem::path;
    using string_list           = ::std::vector< ::std::string >;
    using callback              = ::std::function<void(source_stream&)>;
    using callback_queue        = ::std::vector<callback>;
public:
    source_stream(path const& origin_path, path const& header_dir,
            string_list const& include_dirs);
    ~source_stream();

    void
    open(path const&);

    //@{
    /** @name Work with include files */
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
    //@}

    //@{
    /** @name Scope manipulation */
    void
    start_namespace(::std::string const& name);
    void
    push_scope(::std::string const& name);
    void
    pop_scope();

    void
    at_namespace_scope(callback);

    void
    adjust_scope(qname const& target_scope);
    void
    adjust_scope(ast::entity_ptr ent);

    qname
    current_scope() const;
    //@}

    //@{
    /** @name Output operations */
    void
    write(qname const& qn);
    //@}

    //@{
    /** @name Formatting manipulation */
    void
    write_offset(int temp = 0);
    void
    modify_offset(int delta);
    void
    set_offset(int offset);
    int
    get_offset() const
    { return current_offset_; }
    //@}
private:
    template < typename T >
    friend source_stream&
    operator << (source_stream& os, T const& v);
    friend void
    abs_name(source_stream&);

    path            origin_path_;
    path            header_dir_;
    string_list     include_dirs_;

    ::std::ofstream stream_;

    bool            is_header_;
    ::std::string   header_guard_;

    qname           current_namespace_;
    qname           scope_;
    callback_queue  at_namespace_scope_;

    int             current_offset_;
    int             tab_width_;

    bool            abs_name_;
};

struct offset_guard {
    source_stream&  stream;
    int             init;

    offset_guard(source_stream& ss) : stream{ss}, init{ss.get_offset()} {}
    ~offset_guard() { stream.set_offset(init); }
};

class code_snippet {
public:
    using line  = ::std::pair< int, ::std::string >;
    using lines = ::std::vector< line >;
public:
    code_snippet(qname const& scope)
        : current_scope_{scope}, current_offset_{0}, current_mod_{0}, abs_name_{false}
    {}

    void
    write(qname const& qn);

    void
    write_offset(int temp = 0);
    void
    modify_offset(int delta);

private:
    template < typename T >
    friend code_snippet&
    operator << (code_snippet&, T const&);

    friend void
    abs_name(code_snippet&);
    friend source_stream&
    operator << (source_stream& os, code_snippet const&);

    qname                   current_scope_;
    int                     current_offset_;
    int                     current_mod_;
    ::std::ostringstream    os_;
    lines                   lines_;
    bool                    abs_name_;
};

//@{
/** @name Output operations */
template < typename T >
source_stream&
operator << (source_stream& os, T const& v)
{
    os.stream_ << v;
    return os;
}

inline source_stream&
operator << (source_stream& os, qname const& v)
{
    os.write(v);
    return os;
}

source_stream&
operator << (source_stream& os, mapped_type const& v);

template < typename T >
code_snippet&
operator << (code_snippet& os, T const& v)
{
    os.os_ << v;
    return os;
}

code_snippet&
operator << (code_snippet& os, mapped_type const& v);

source_stream&
operator << (source_stream& os, grammar::data_initializer const& init);

inline code_snippet&
operator << (code_snippet& os, qname const& v)
{
    os.write(v);
    return os;
}

source_stream&
operator << (source_stream& os, code_snippet const&);

//@}

//@{
/** @name Stream manipulators */
inline void
off(source_stream& ss)
{
    ss.write_offset();
}

inline void
off(code_snippet& ss)
{
    ss.write_offset();
}

inline void
abs_name(source_stream& ss)
{
    ss.abs_name_ = true;
}

inline void
abs_name(code_snippet& ss)
{
    ss.abs_name_ = true;
}


inline source_stream&
operator << (source_stream& os, void(*f)(source_stream&))
{
    f(os);
    return os;
}

inline code_snippet&
operator << (code_snippet& os, void(*f)(code_snippet&))
{
    f(os);
    return os;
}

struct offset_change {
    int delta;
};

inline source_stream&
operator << (source_stream& os, offset_change const& v)
{
    os.modify_offset(v.delta);
    os.write_offset();
    return os;
}

inline code_snippet&
operator << (code_snippet& os, offset_change const& v)
{
    os.modify_offset(v.delta);
    os.write_offset();
    return os;
}

inline offset_change
mod(int delta)
{
    return {delta};
}

struct temp_offset {
    int delta;
};

inline source_stream&
operator << (source_stream& os, temp_offset const& v)
{
    os.write_offset(v.delta);
    return os;
}

inline code_snippet&
operator << (code_snippet& os, temp_offset const& v)
{
    os.write_offset(v.delta);
    return os;
}


inline temp_offset
off(int delta)
{
    return {delta};
}


//@}

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_WIRE2CPP_CPP_SOURCE_STREAM_HPP_ */
