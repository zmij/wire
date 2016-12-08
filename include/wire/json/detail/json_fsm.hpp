/*
 * json_fsm.hpp
 *
 *  Created on: 3 дек. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_JSON_FSM_HPP_
#define WIRE_JSON_DETAIL_JSON_FSM_HPP_

#include <afsm/fsm.hpp>
#include <wire/json/json_io_base.hpp>

namespace wire {
namespace json {
namespace detail {

namespace events {

//@{
/** @name Punctuation */
struct comma {};
struct colon {};
struct eol {};
//@}

struct null_literal {};

struct start_object {};
struct end_object {};

struct start_array {};
struct end_array {};

}  /* namespace events */

enum class value_context {
    none,
    array,
    array_first,
    object
};

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct json_parser_fsm_def
        : ::afsm::def::state_machine< json_parser_fsm_def<CharT, Traits>> {

    using fsm_type          = ::afsm::state_machine< json_parser_fsm_def<CharT, Traits> >;
    using json_io           = json_io_base<CharT, Traits>;
    using string_type       = typename json_io::string_type;
    using integral_type     = typename json_io::integral_type;
    using float_type        = typename json_io::float_type;

    //@{
    /** @name Pull type aliases from base type */
    using definition_type   = ::afsm::def::state_machine< json_parser_fsm_def<CharT, Traits> >;

    using none              = typename definition_type::none;

    template < typename T, typename ... U >
    using state_machine     = typename definition_type::template state_machine<T, U...>;
    template < typename T, typename ... U >
    using state             = typename definition_type::template state<T, U...>;
    template < typename T, typename M, typename ... U >
    using push              = typename definition_type::template push<T, M, U...>;
    template < typename T, typename M, typename ... U >
    using pop               = typename definition_type::template pop<T, M, U...>;

    template < typename ... U >
    using transition_table  = typename definition_type::template transition_table<U...>;
    template < typename S, typename E, typename T, typename A = none, typename G = none >
    using tr                = typename definition_type::template tr<S, E, T, A, G>;
    template < typename E, typename A = none, typename G = none >
    using in                = typename definition_type::template in<E, A, G>;

    template < typename ... U >
    using type_tuple        = typename definition_type::template type_tuple<U...>;
    //@}

    struct context : state_machine< context > {
        //@{
        /** @name Substates */
        struct start : state< start > {
            template < typename FSM >
            void
            on_enter(events::start_array&&, FSM& fsm)
            {
                fsm.context_ = value_context::array_first;
            }
            template < typename FSM >
            void
            on_enter(events::comma&&, FSM& fsm)
            {
                fsm.context_ = value_context::array;
            }
            template < typename FSM >
            void
            on_enter(events::colon&&, FSM& fsm)
            {
                fsm.context_ = value_context::object;
            }
        };
        struct array : state_machine< array > {
            struct value : push< value, json_parser_fsm_def > {};

            using initial_state = value;

            using transitions = transition_table<
                tr< value,  events::comma,          value >
            >;
        };
        struct object : state_machine< object > {
            struct name : state<name> {
                template < typename FSM >
                void
                on_exit(string_type const&, FSM& fsm)
                {
                    ++fsm.size;
                }
            };
            struct colon : state< colon > {};
            struct value : push< value, json_parser_fsm_def > {};

            using initial_state = name;

            using transitions = transition_table<
                tr< name,   string_type,            colon   >,
                tr< colon,  events::colon,          value   >,
                tr< value,  events::comma,          name    >
            >;
            ::std::size_t   size    = 0;
        };
        struct end : pop< end, json_parser_fsm_def > {};
        //@}
        //@{
        /** @name Guards */
        template < value_context Ctx >
        struct in_context {
            template < typename FSM, typename State >
            bool
            operator()(FSM const& fsm, State const&) const
            {
                return fsm.context_ == Ctx;
            }
        };
        using in_first_element = in_context<value_context::array_first>;
        struct can_close_object {
            template < typename FSM, typename State >
            bool
            operator()(FSM const& fsm, State const& state) const
            {
                return (state.size == 0 && fsm.template is_in_state< typename object::name >())
                        || fsm.template is_in_state< typename object::value >();
            }
        };
        //@}
        //@{
        /** @name Actions */
        struct dup_event {
            template < typename Event, typename FSM >
            void
            operator()(Event&& evt, FSM& fsm)
            {
                root_machine(fsm).process_event(::std::forward<Event>(evt));
            }
        };
        //@}

        using initial_state = start;
        using transitions = transition_table<
            /*  Start | Event                     | Next  | Action    | Guard               */
            /*--------+---------------------------+-------+-----------+---------------------*/
            tr< start , bool                      , end   , none      , none                >,
            tr< start , integral_type             , end   , none      , none                >,
            tr< start , float_type                , end   , none      , none                >,
            tr< start , string_type               , end   , none      , none                >,

            tr< start , events::null_literal      , end   , none      , none                >,
            /*--------+---------------------------+-------+-----------+---------------------*/
            tr< start , events::start_array       , array , none      , none                >,
            tr< array , events::end_array         , end   , none      , none                >,
            tr< start , events::end_array         , end   , dup_event , in_first_element    >,

            /*--------+---------------------------+-------+-----------+---------------------*/
            tr< start , events::start_object      , object, none      , none                >,
            tr< object, events::end_object        , end   , none      , can_close_object    >
        >;
        value_context   context_ = value_context::none;
    };
    using orthogonal_regions = type_tuple<context>;
    using internal_transitions = transition_table<
        // Track position
        in< events::eol >
    >;

    fsm_type&
    rebind()
    { return static_cast<fsm_type&>(*this); }
    fsm_type const&
    rebind() const
    { return static_cast<fsm_type const&>(*this); }

    template <typename Token>
    bool
    operator()(Token const& tok)
    {
        using chars     = typename json_io::chars;
        using char_type = typename json_io::char_type;
        auto& fsm = rebind();
        if (tok.id() < json_io::id_string) {
            char c = tok.id();
            ::std::cerr << "Token value '" << c << "'\n";
        } else if (tok.id() < json_io::id_newline) {
            ::std::cerr << "Token id " << tok.id()
                << " value '" << tok.value() << "' which "
                << tok.value().which() << "\n";
        }
        switch (tok.id()) {
            case json_io::cvt(chars::start_object):
                return ok(fsm.process_event(events::start_object{}));
            case json_io::cvt(chars::end_object):
                return ok(fsm.process_event(events::end_object{}));
            case json_io::cvt(chars::start_array):
                return ok(fsm.process_event(events::start_array{}));
            case json_io::cvt(chars::end_array):
                return ok(fsm.process_event(events::end_array{}));
            case json_io::cvt(chars::colon):
                return ok(fsm.process_event(events::colon{}));
            case json_io::cvt(chars::comma):
                return ok(fsm.process_event(events::comma{}));
            case json_io::id_string:
                return ok(fsm.process_event(string_type{}));
            case json_io::id_empty_string:
                return ok(fsm.process_event(string_type{}));
            case json_io::id_integral:
//                return ok(fsm.process_event(
//                        ::boost::get<integral_type>(tok.value())));
                return ok(fsm.process_event(integral_type{}));
            case json_io::id_float:
                return ok(fsm.process_event(float_type{}));
//                return ok(fsm.process_event(
//                        ::boost::get<float_type>(tok.value())));
            case json_io::id_true:
                return ok(fsm.process_event(true));
            case json_io::id_false:
                return ok(fsm.process_event(false));
            case json_io::id_null:
                return ok(fsm.process_event(events::null_literal{}));
            case json_io::id_newline:
            case json_io::id_ws:
                return true;
            default:
                ::std::cerr << "Unknown token type " << tok.id() << "\n";
                break;
        }

        return false;
    }
};

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
using basic_json_fsm = ::afsm::state_machine< json_parser_fsm_def<CharT, Traits> >;

using json_fsm = basic_json_fsm<char>;
using wjson_fsm = basic_json_fsm<wchar_t>;

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */

namespace afsm {
extern template class state_machine< wire::json::detail::json_parser_fsm_def<char> >;
extern template class state_machine< wire::json::detail::json_parser_fsm_def<wchar_t> >;
}  /* namespace afsm */

#endif /* WIRE_JSON_DETAIL_JSON_FSM_HPP_ */
