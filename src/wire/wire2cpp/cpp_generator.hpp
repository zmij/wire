/*
 * cpp_generator.hpp
 *
 *  Created on: 27 апр. 2016 г.
 *      Author: sergey.fedorov
 */

/**
 * @page wire2cpp Wire to C++ mapping
 *
 *  ## Annotations
 *
 *  ### cpp_container
 *
 *  Applicable to sequence and dictionary
 *
 */

#ifndef WIRE_WIRE2CPP_CPP_GENERATOR_HPP_
#define WIRE_WIRE2CPP_CPP_GENERATOR_HPP_

#include <wire/idl/ast.hpp>
#include <wire/idl/preprocess.hpp>
#include <wire/idl/generator.hpp>
#include <wire/idl/grammar/declarations.hpp>

#include <fstream>
#include <deque>
#include <functional>

namespace wire {
namespace idl {
namespace cpp {

namespace annotations {

::std::string const CPP_CONTAINER = "cpp_container";

}  /* namespace annotations */

struct generate_options {
    ::std::string    header_include_dir;

    ::std::string    header_output_dir;
    ::std::string    source_output_dir;
};

struct offset {
    ::std::size_t sz = 0;

    offset&
    operator++ ()
    {
        ++sz;
        return *this;
    }

    offset
    operator++ (int)
    {
        offset tmp{*this};
        ++sz;
        return tmp;
    }

    offset&
    operator-- ()
    {
        if (sz > 0)
            --sz;
        return *this;
    }

    offset
    operator-- (int)
    {
        offset tmp{*this};
        if (sz > 0)
            --sz;
        return tmp;
    }

    offset&
    operator += (::std::size_t off)
    {
        sz += off;
        return *this;
    }

    offset&
    operator -= (::std::size_t off)
    {
        if (sz < off)
            sz = 0;
        else
            sz -= off;
        return *this;
    }
};

inline offset
operator + (offset const& off, ::std::size_t sz)
{
    offset tmp{off};
    tmp += sz;
    return tmp;
}

class generator : public ast::generator {
public:
    using include_dir_list = ::std::vector< std::string >;
public:
    generator(generate_options const&, preprocess_options const&,
            ast::global_namespace_ptr);
    virtual ~generator();

    void
    generate_constant( ast::constant_ptr c) override;

    void
    generate_type_alias( ast::type_alias_ptr ta ) override;

    void
    generate_enum(ast::enumeration_ptr enum_) override;

    void
    generate_struct(ast::structure_ptr struct_) override;

    void
    generate_interface(ast::interface_ptr iface);

    void
    generate_class(ast::class_ptr class_);

    void
    generate_exception(ast::exception_ptr exc) override;
private:
    void
    adjust_scope(qname_search const& qn);
    void
    adjust_scope(ast::entity_ptr en);
    void
    pop_scope();

    ::std::string
    constant_prefix( qname const& ) const;

    ::std::ostream&
    write_type_name(::std::ostream&, ast::type_const_ptr t,
            grammar::annotation_list const& = grammar::annotation_list{});

    ::std::ostream&
    write_qualified_name(::std::ostream&, qname const& qn);

    ::std::ostream&
    write_init(::std::ostream&, offset& off, grammar::data_initializer const& init);

    ::std::ostream&
    write_data_member(::std::ostream&, offset const&, ast::variable_ptr var);

    void
    generate_read_write( ast::structure_ptr struct_);
private:
    using free_function = ::std::function< void() >;
private:
    ast::global_namespace_ptr       ns_;
    ast::compilation_unit_ptr       unit_;

    ::std::ofstream                 header_;
    ::std::ofstream                 source_;

    offset                          h_off_;
    offset                          s_off_;

    ::std::string                   header_guard_;

    qname                           current_scope_;

    ::std::deque<ast::type_ptr>     scope_stack_;
    ::std::vector< free_function >  free_functions_;
};

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_WIRE2CPP_CPP_GENERATOR_HPP_ */
