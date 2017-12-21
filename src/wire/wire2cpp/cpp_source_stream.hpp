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
    pop_to_namespace();

    void
    at_namespace_scope(callback);

    void
    adjust_namespace(ast::qname const& target_scope);
    void
    adjust_namespace(ast::entity_ptr ent);

    ast::qname
    current_scope() const;

    ast::qname const&
    current_namespace() const
    { return current_namespace_; }
    //@}

    //@{
    /** @name Output operations */
    void
    write(ast::qname const& qn);
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

    ast::qname      current_namespace_;
    ast::qname      scope_;
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
    code_snippet(ast::qname const& scope)
        : current_scope_{scope}, current_offset_{0}, current_mod_{0}, abs_name_{false}
    {}

    void
    write(ast::qname const& qn);

    void
    write_offset(int temp = 0);
    void
    modify_offset(int delta);

    ast::qname
    current_scope() const
    { return current_scope_; }

    ::std::string
    str(int tab_width = 4) const;
private:
    template < typename T >
    friend code_snippet&
    operator << (code_snippet&, T const&);

    friend void
    abs_name(code_snippet&);
    friend source_stream&
    operator << (source_stream& os, code_snippet const&);
    friend code_snippet&
    operator << (code_snippet& os, code_snippet const&);

    ast::qname              current_scope_;
    int                     current_offset_;
    int                     current_mod_;
    ::std::ostringstream    os_;
    lines                   lines_;
    bool                    abs_name_;
};

inline ast::qname
qname(ast::entity_const_ptr en)
{
    return en->get_qualified_name();
}

inline ast::qname
qname(ast::enumeration_const_ptr enum_, ::std::string const& enumerator)
{
    ast::qname qn = enum_->get_qualified_name();
    if (!enum_->constrained()) {
        qn.components.pop_back();
    }
    qn.components.push_back(enumerator);
    return qn;
}

void
strip_quotes(::std::string& str);

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
operator << (source_stream& os, ast::qname const& v)
{
    os.write(v);
    return os;
}

source_stream&
operator << (source_stream& os, mapped_type const& v);
source_stream&
operator << (source_stream& os, invoke_param const& v);

template < typename T >
code_snippet&
operator << (code_snippet& os, T const& v)
{
    os.os_ << v;
    return os;
}

code_snippet&
operator << (code_snippet& os, mapped_type const& v);
code_snippet&
operator << (code_snippet& os, invoke_param const& v);

source_stream&
operator << (source_stream& os, grammar::data_initializer const& init);

inline code_snippet&
operator << (code_snippet& os, ast::qname const& v)
{
    os.write(v);
    return os;
}

source_stream&
operator << (source_stream& os, code_snippet const&);

code_snippet&
operator << (code_snippet& os, code_snippet const& cs);

struct cpp_safe_name {
    ::std::string const& name;
};
struct cpp_safe_qname {
    ast::qname const& qn;
};
struct cpp_safe_qname_search {
    ast::qname_search qs;
};

inline cpp_safe_name
cpp_name(::std::string const& name)
{
    return { name };
}

inline cpp_safe_name
cpp_name(ast::entity_const_ptr const& ent)
{
    return cpp_name(ent->name());
}

inline cpp_safe_qname
cpp_name(ast::qname const& qn)
{
    return { qn };
}

inline cpp_safe_qname_search
cpp_name(ast::qname_search qs)
{
    return { qs };
}

::std::ostream&
operator << (::std::ostream& os, cpp_safe_name const& val);
::std::ostream&
operator << (::std::ostream& os, cpp_safe_qname const& val);
::std::ostream&
operator << (::std::ostream& os, cpp_safe_qname_search const& val);
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
