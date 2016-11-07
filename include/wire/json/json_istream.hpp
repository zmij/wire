/*
 * json_istream.hpp
 *
 *  Created on: 5 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_JSON_ISTREAM_HPP_
#define WIRE_JSON_JSON_ISTREAM_HPP_

#include <string>
#include <locale>
#include <iostream>
#include <stack>
#include <type_traits>
#include <iomanip>

#include <wire/json/json_stream_fwd.hpp>
#include <wire/json/json_io_base.hpp>
#include <wire/json/traits.hpp>
#include <wire/json/parser.hpp>

namespace wire {
namespace json {

namespace detail {

template < typename CharT, typename Traits, traits::value_type >
struct value_input_wrapper_impl {
    using json_stream_type  = basic_json_istream<CharT, Traits>;
    using basic_parser_type = basic_parser_base<CharT, Traits>;
    using parser_ptr_type   = typename basic_parser_type::parser_ptr_type;
    template < typename T >
    using parser = basic_parser<T, CharT, Traits>;

    template < typename T >
    static void
    before(json_stream_type&, json_context, T const&) {}

    template < typename T >
    static parser_ptr_type
    create_parser(T& val)
    {
        using parser_type = parser<T>;
        return ::std::make_shared<parser_type>(val);
    }
    template < typename T >
    static void
    input(json_stream_type& is, T& val)
    {
        // Call read_json here
    }
    template < typename T >
    static void
    after(json_stream_type&, json_context, T const&) {}
};

template < typename CharT, typename Traits >
struct value_input_wrapper_impl<CharT, Traits, traits::value_type::OBJECT> {
    using json_stream_type  = basic_json_istream<CharT, Traits>;
    using basic_parser_type = basic_parser_base<CharT, Traits>;
    using parser_ptr_type   = typename basic_parser_type::parser_ptr_type;
    template < typename T >
    using parser = basic_parser<T, CharT, Traits>;

    template < typename T >
    static void
    before(json_stream_type& is, json_context, T const&)
    { is.start_object(); }
    template < typename T >
    static parser_ptr_type
    create_parser(T& val)
    { return parser_ptr_type{}; }
    template < typename T >
    static void
    input(json_stream_type& is, T& val)
    { json_read(is, val); }
    template < typename T >
    static void
    after(json_stream_type& is, json_context, T const&)
    { is.end_object(); }
};


}  /* namespace detail */

template < typename CharT, typename Traits, typename T >
struct value_input_wrapper :
    detail::value_input_wrapper_impl< CharT, Traits, traits::json_type<T>::value > {};

template < typename CharT, typename Traits >
class basic_json_istream : public json_io_base<CharT, Traits> {
public:
    using base_type     = json_io_base<CharT, Traits>;
    using char_type     = typename base_type::char_type;
    using traits_type   = typename base_type::traits_type;
    using this_type     = basic_json_istream<char_type, traits_type>;
    using stream_type   = typename base_type::istream_type;
    using string_type   = typename base_type::string_type;
    using chars         = typename base_type::chars;
    using size_type     = ::std::size_t;
    template < typename T >
    using parser        = basic_parser<T, char_type, traits_type>;
    using basic_parser  = detail::basic_parser_base<char_type, traits_type>;
    using root_parser   = detail::basic_root_parser<char_type, traits_type>;
    using parser_ptr    = typename basic_parser::parser_ptr_type;
    template < typename T >
    using input_wrapper = value_input_wrapper<char_type, traits_type, T>;
    using object_parser = detail::basic_object_parser<char_type, traits_type>;
public:
    basic_json_istream(stream_type& is)
        : is_{is} {}
    basic_json_istream(basic_json_istream const&) = delete;
    basic_json_istream(basic_json_istream&&) = default;
    ~basic_json_istream() {}

    this_type&
    read(char_type const* member_name)
    {
        if (current_context() != json_context::object)
            throw ::std::runtime_error{"Cannot add a member name at this context"};
        current_state().next_name = string_type{member_name};
        push_context(json_context::value);
        return *this;
    }
    template <typename T>
    this_type&
    read(T& val)
    {
        using value_type    = typename ::std::decay<T>::type;
        using wrapper_type  = input_wrapper<value_type>;
        switch (current_context()) {
            case json_context::none:
                if (current_state().size > 0)
                    throw ::std::runtime_error{"Cannot have more than one object at root level"};
                ++current_state().size;
                break;
            default:
                break;
        }

        auto ctx = current_context();
        wrapper_type::before(*this, ctx, val);
        auto p = wrapper_type::create_parser(val);
        if (p) {
            // TODO Check context and add either an element parser or
            //      member parser
            current_state().parser->add_element_parser(p);
        }
        wrapper_type::input(*this, val);
        wrapper_type::after(*this, ctx, val);

        if (current_context() == json_context::none) {
            if (!current_state().parser)
                throw ::std::runtime_error{"Parser is not initialized"};
            detail::parse(*current_state().parser, is_);
        }
        return *this;
    }

    this_type&
    start_object(parser_ptr p = parser_ptr{})
    {
        if (current_context() == json_context::value_key)
            throw ::std::runtime_error{"json start object: invalid state"};
        if (!p) {
            p = ::std::make_shared<object_parser>();
        }
        if (current_context() == json_context::value) {
            // Set member subparser
            if (current_state().next_name.empty()) {
                throw ::std::runtime_error{"Name for member parser is not set"};
            }
            pop_context();
            current_state().parser->add_member_parser(::std::move(current_state().next_name), p);
            current_state().next_name.clear();
        } else {
            current_state().parser->add_element_parser(p);
        }
        push_context(json_context::object, p);
        return *this;
    }
    this_type&
    end_object()
    {
        if (current_context() != json_context::object)
            throw ::std::runtime_error{"json end object: invalid state"};
        pop_context();
        return *this;
    }
    this_type&
    start_array()
    {
        return *this;
    }
    this_type&
    end_array()
    {
        return *this;
    }

    ::std::locale
    getloc() const
    { return is_.getloc(); }
    void
    imbue( ::std::locale const& loc )
    { is_.imbue(loc); }
    template < typename Facet >
    void
    add_facet(Facet* fct)
    { is_.imbue(::std::locale{is_.getloc(), fct}); }

    stream_type&
    stream()
    { return is_; }

    void
    push_context(json_context ctx, parser_ptr p = parser_ptr{})
    { state_.emplace(state_type{ctx, size_type{0}, p, {}}); }
    void
    pop_context()
    { if (!state_.empty()) state_.pop(); }
    json_context
    current_context() const
    { return current_state().context; }
private:
    struct state_type {
        json_context        context;
        size_type           size;
        parser_ptr          parser;
        string_type         next_name;
    };
    using state_stack   = ::std::stack<state_type>;

    state_type&
    current_state()
    {
        if (state_.empty())
            return none_;
        return state_.top();
    }
    state_type const&
    current_state() const
    {
        if (state_.empty())
            return none_;
        return state_.top();
    }
private:
    stream_type&        is_;
    state_type          none_{ json_context::none, 0,
        ::std::make_shared<root_parser>(), nullptr };
    state_stack         state_;
};

extern template class basic_json_istream<char>;
extern template class basic_json_istream<wchar_t>;

template < typename CharT, typename Traits >
basic_json_istream<CharT, Traits>&
operator >> (basic_json_istream<CharT, Traits>& is, void(*manip)(basic_json_istream<CharT, Traits>&))
{
    manip(is);
    return is;
}

template < typename CharT, typename Traits, typename T >
basic_json_istream<CharT, Traits>&
operator >> (basic_json_istream<CharT, Traits>& is, T&& val)
{
    return is.read(::std::forward<T>(val));
}

//----------------------------------------------------------------------------
template < typename CharT, typename Traits >
void
start_object(basic_json_istream<CharT, Traits>& is)
{
    is.start_object();
}

template < typename CharT, typename Traits >
void
end_object(basic_json_istream<CharT, Traits>& is)
{
    is.end_object();
}

template < typename CharT, typename Traits >
void
start_array(basic_json_istream<CharT, Traits>& is)
{
    is.start_array();
}

template < typename CharT, typename Traits >
void
end_array(basic_json_istream<CharT, Traits>& is)
{
    is.end_array();
}

}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_JSON_ISTREAM_HPP_ */
