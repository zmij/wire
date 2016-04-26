/*
 * declarations.hpp
 *
 *  Created on: Apr 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_
#define WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_

#include <wire/idl/type_name.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <wire/idl/grammar/source_advance.hpp>

namespace wire {
namespace idl {
namespace grammar {

using type_alias_decl  = ::std::pair< ::std::string, type_name >; // TODO declaration location
using fwd_decl = ::std::pair< ::std::string, ::std::string >;
using type_name_list = ::std::vector< type_name >;

//----------------------------------------------------------------------------
struct data_initializer {
    using initializer_ptr   = ::std::shared_ptr< data_initializer >;
    using initializer_list  = ::std::vector< initializer_ptr >;
    using initializer_value = ::boost::variant< ::std::string, initializer_list >;

    initializer_value       value;
};

//----------------------------------------------------------------------------
struct data_member_decl {
    using optional_init = ::boost::optional< data_initializer >;

    type_name               type;
    ::std::string           name;
    optional_init           init;
};

//----------------------------------------------------------------------------
struct function_decl {
    using param_type = ::std::pair< type_name, ::std::string >;
    using params_list = ::std::vector< param_type >;
    using type_list = ::std::vector< type_name >;

    type_name               return_type;
    ::std::string           name;
    params_list             params;
    bool                    const_qualified;
    type_list               throw_spec;
};

//----------------------------------------------------------------------------
struct enumerator_decl {
    using optional_initializer = ::boost::optional< ::std::string >;
    ::std::string           name;
    optional_initializer    init;
};

//----------------------------------------------------------------------------
struct enum_decl {
    using enumerator_list = ::std::vector< enumerator_decl >;
    ::std::string           name;
    bool                    constrained;
    enumerator_list         enumerators;
};

//----------------------------------------------------------------------------
struct annotation {

};

//----------------------------------------------------------------------------
template < typename State >
struct parser_state_update {
    using state_type = State;
    template < typename Func >
    using fn = ::boost::phoenix::function< Func >;

    struct update_source_location_func {
        using result = void;
        template < typename TokenValue >
        void
        operator()(TokenValue const& tok) const
        {
            namespace qi = ::boost::spirit::qi;
            using Iterator = decltype(tok.begin());
            using grammar_type = grammar::location_grammar<Iterator>;

            auto begin = tok.begin();
            auto end = tok.end();

            source_location loc;

            if (!qi::parse(begin, end, grammar_type{}, loc) || begin != end) {
                throw ::std::runtime_error("Invalid preprocessor location string");
            }
            state.update_location(end, loc);
        }
        state_type& state;
    };

    struct add_type_alias_func {
        using result = void;
        void
        operator()( type_alias_decl const& decl ) const
        {
            state.add_type_alias(decl);
        }
        state_type& state;
    };

    struct add_forward_decl_func {
        using result = void;
        void
        operator()( fwd_decl const& decl ) const
        {
            state.forward_declare(decl);
        }
        state_type& state;
    };

    struct add_enum_decl_func {
        using result = void;
        void
        operator()(enum_decl const& decl) const
        {
            state.declare_enum(decl);
        }
        state_type& state;
    };

    struct add_constant_func {
        using result = void;
        void
        operator()(data_member_decl const& decl) const
        {
            state.add_constant(decl);
        }
        state_type& state;
    };
    struct add_data_member_func {
        using result = void;
        void
        operator()(data_member_decl const& decl) const
        {
            state.add_data_member(decl);
        }
        state_type& state;
    };
    struct add_func_member_func {
        using result = void;
        void
        operator()(function_decl const& decl) const
        {
            state.add_func_member(decl);
        }
        state_type& state;
    };

    struct end_scope_func {
        using result = void;
        void
        operator()() const
        {
            state.end_scope();
        }
        state_type& state;
    };

    struct start_namespace_func {
        using result = void;
        void
        operator()(::std::string const& name) const
        {
            state.start_namespace(name);
        }
        state_type& state;
    };

    struct start_struct_func {
        using result = void;
        void
        operator()(::std::string const& name) const
        {
            state.start_structure(name);
        }
        state_type& state;
    };

    struct start_interface_func {
        using result = void;
        using optional_type_list = ::boost::optional< type_name_list >;
        void
        operator()(::std::string const& name, optional_type_list const& ancestors) const
        {
            state.start_interface(name, ancestors);
        }
        state_type& state;
    };

    struct start_class_func {
        using result = void;
        using optional_type_list = ::boost::optional< type_name_list >;
        void
        operator()(::std::string const& name, optional_type_list const& ancestors) const
        {
            state.start_class(name, ancestors);
        }
        state_type& state;
    };

    struct start_exception_func {
        using result = void;
        using optional_type = ::boost::optional< type_name >;
        void
        operator()(::std::string const& name, optional_type const& ancestor) const
        {
            state.start_exception(name, ancestor);
        }
        state_type& state;
    };

    parser_state_update(state_type& st)
        : state(st),
          location          { update_source_location_func{st} },
          add_type_alias    { add_type_alias_func{st} },
          forward_declare   { add_forward_decl_func{st} },
          declare_enum      { add_enum_decl_func{st} },
          add_constant      { add_constant_func{st} },
          add_data_member   { add_data_member_func{st} },
          add_func_member   { add_func_member_func{st} },
          start_namespace   { start_namespace_func{st} },
          start_structure   { start_struct_func{st} },
          start_interface   { start_interface_func{st} },
          start_class       { start_class_func{st} },
          start_exception   { start_exception_func{st} },
          end_scope         { end_scope_func{st} }
    {
    }

    state_type&                         state;
    fn< update_source_location_func >   location;
    fn< add_type_alias_func >           add_type_alias;
    fn< add_forward_decl_func >         forward_declare;
    fn< add_enum_decl_func >            declare_enum;
    fn< add_constant_func >             add_constant;
    fn< add_data_member_func >          add_data_member;
    fn< add_func_member_func >          add_func_member;
    fn< start_namespace_func >          start_namespace;
    fn< start_struct_func >             start_structure;
    fn< start_interface_func >          start_interface;
    fn< start_class_func >              start_class;
    fn< start_exception_func >          start_exception;
    fn< end_scope_func >                end_scope;
};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_ */
