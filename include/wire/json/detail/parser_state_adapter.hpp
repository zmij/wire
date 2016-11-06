/*
 * parser_state_adapter.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_PARSER_STATE_ADAPTER_HPP_
#define WIRE_JSON_DETAIL_PARSER_STATE_ADAPTER_HPP_

#include <wire/idl/grammar/common.hpp>
#include <wire/json/json_io_base.hpp>

namespace wire {
namespace json {
namespace grammar {

template < typename State, typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_parser_state_adapter {
    using state_type    = State;
    using json_io_type  = json_io_base<CharT, Traits>;
    using string_type   = typename json_io_type::string_type;

    template < typename Func >
    using fn = ::boost::phoenix::function< Func >;

    //@{
    /** @name Literals */
    struct string_literal_func {
        using result = void;
        state_type& state;

        void
        operator()(string_type const& tok) const
        {
            state.string_literal(tok);
        }
    };

    struct integral_literal_func {
        using result = void;
        state_type& state;

        void
        operator()(::std::int64_t val) const
        {
            state.integral_literal(val);
        }
    };

    struct float_literal_func {
        using result = void;
        state_type& state;

        void
        operator()(long double val) const
        {
            state.float_literal(val);
        }
    };

    struct bool_literal_func {
        using result = void;
        state_type& state;

        void
        operator()(bool val) const
        {
            state.bool_literal(val);
        }
    };

    struct null_literal_func {
        using result = void;
        state_type& state;

        void
        operator()() const
        {
            state.null_literal();
        }
    };
    //@}

    struct start_array_func {
        using result = void;
        state_type& state;

        void
        operator()() const
        {
            state.start_array();
        }
    };
    struct end_array_func {
        using result = void;
        state_type& state;

        void
        operator()() const
        {
            state.end_array();
        }
    };
    struct start_element_func {
        using result = void;
        state_type& state;

        void
        operator()() const
        {
            state.start_element();
        }
    };

    struct start_object_func {
        using result = void;
        state_type& state;

        void
        operator()() const
        {
            state.start_object();
        }
    };
    struct end_object_func {
        using result = void;
        state_type& state;

        void
        operator()() const
        {
            state.end_object();
        }
    };
    struct start_member_func {
        using result = void;
        state_type& state;

        void
        operator()(string_type const& name) const
        {
            state.start_member(name);
        }
    };


    basic_parser_state_adapter(state_type& st)
        : state{st},
          string_literal    { string_literal_func{st} },
          integral_literal  { integral_literal_func{st} },
          float_literal     { float_literal_func{st} },
          bool_literal      { bool_literal_func{st} },
          null_literal      { null_literal_func{st} },
          start_array       { start_array_func{st} },
          end_array         { end_array_func{st} },
          start_element     { start_element_func{st} },
          start_object      { start_object_func{st} },
          end_object        { end_object_func{st} },
          start_member      { start_member_func{st} }
    {
    }

    state_type&                 state;
    //@{
    /** @name Literals */
    fn< string_literal_func >   string_literal;
    fn< integral_literal_func > integral_literal;
    fn< float_literal_func >    float_literal;
    fn< bool_literal_func >     bool_literal;
    fn< null_literal_func >     null_literal;
    //@}
    //@{
    /** @name Scopes */
    fn< start_array_func >      start_array;
    fn< end_array_func >        end_array;
    fn< start_element_func >    start_element;
    fn< start_object_func >     start_object;
    fn< end_object_func >       end_object;
    fn< start_member_func >     start_member;
    //@}
};

}  /* namespace grammar */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_PARSER_STATE_ADAPTER_HPP_ */
