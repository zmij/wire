/*
 * json_stream.hpp
 *
 *  Created on: 4 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_JSON_STREAM_HPP_
#define WIRE_JSON_JSON_STREAM_HPP_

#include <string>
#include <locale>
#include <iostream>
#include <stack>
#include <type_traits>
#include <iomanip>

#include <wire/json/traits.hpp>

namespace wire {
namespace json {

enum class json_context : ::std::uint8_t {
    none,       /**< JSON output hasn't been started */
    value_key,  /**< A value key is required here (object scope) */
    value,      /**< A value is required here (after a value key, object scope) */
    object,     /**< A value key is required here (object scope) */
    array,      /**< A value is required here (array scope) */
};

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct json_io_base {
    using char_type     = CharT;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    enum class chars {
        null,
        double_quote,
        escape_symbol,
        start_object,
        end_object,
        start_array,
        end_array,
        colon,
        comma,
        space,
        newline,
    };
    static constexpr char_type* true_str    = nullptr;
    static constexpr char_type* false_str   = nullptr;
    static constexpr char_type* null_str    = nullptr;

    static ostream_type&
    put(ostream_type& os, chars)
    { return os; }
};

template < typename Traits >
struct json_io_base<char, Traits> {
    using char_type     = char;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    enum class chars : char_type {
        null            = 0,
        double_quote    = '"',
        escape_symbol   = '\\',
        start_object    = '{',
        end_object      = '}',
        start_array     = '[',
        end_array       = ']',
        colon           = ':',
        comma           = ',',
        space           = ' ',
        newline         = '\n',
    };
    static constexpr char_type const* true_str  = "true";
    static constexpr char_type const* false_str = "false";
    static constexpr char_type const* null_str  = "null";

    static ostream_type&
    put(ostream_type& os, chars v)
    {
        os.put(static_cast<char_type>(v));
        return os;
    }
};

template < typename Traits >
struct json_io_base<wchar_t, Traits> {
    using char_type     = wchar_t;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    enum class chars : char_type {
        null            = 0,
        double_quote    = L'"',
        escape_symbol   = L'\\',
        start_object    = L'{',
        end_object      = L'}',
        start_array     = L'[',
        end_array       = L']',
        colon           = L':',
        comma           = L',',
        space           = L' ',
        newline         = L'\n',
    };
    static constexpr char_type const* true_str  = L"true";
    static constexpr char_type const* false_str = L"false";
    static constexpr char_type const* null_str  = L"null";

    static ostream_type&
    put(ostream_type& os, chars v)
    {
        os.put(static_cast<char_type>(v));
        return os;
    }
};

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
class basic_json_ostream;

namespace detail {
    template < typename CharT, typename Traits, traits::value_type >
    struct value_output_wrapper_impl {
        using json_stream_type = basic_json_ostream<CharT, Traits>;

        template <typename T>
        static void
        before(json_stream_type&, json_context, T const&) {}
        template <typename T>
        static void
        output(json_stream_type& os, T&& val)
        {
            os.stream() << ::std::forward<T>(val);
        }
        static void
        output(json_stream_type& os, ::std::nullptr_t)
        {
            os.stream() << json_stream_type::null_str;
        }
        static void
        output(json_stream_type& os, bool val)
        {
            os.stream() << (val ? json_stream_type::true_str
                    : json_stream_type::false_str);
        }
        template < typename T >
        static void
        output_value_name(json_stream_type& os, T&& val)
        {
            output(os, ::std::forward<T>(val));
        }
        template < typename T >
        static void
        after(json_stream_type&, json_context, T const&) {}
    };

    template < typename CharT, typename Traits >
    struct value_output_wrapper_impl< CharT, Traits, traits::value_type::STRING > {
        using json_stream_type = basic_json_ostream<CharT, Traits>;

        template < typename T >
        static void
        before(json_stream_type& os, json_context, T const&)
        { os.put(json_stream_type::chars::double_quote); }
        template < typename T >
        static void
        output(json_stream_type& os, T&& val)
        {
            json_write(os, ::std::forward<T>(val));
        }
        template < typename T >
        static void
        output_value_name(json_stream_type& os, T&& val)
        {
            output(os, ::std::forward<T>(val));
        }
        template < typename T >
        static void
        after(json_stream_type& os, json_context, T const&)
        { os.put(json_stream_type::chars::double_quote); }
    };

    template < typename CharT, typename Traits >
    struct value_output_wrapper_impl< CharT, Traits,
            traits::value_type::ARRAY > {
        using json_stream_type = basic_json_ostream<CharT, Traits>;

        template < typename T >
        static void
        before(json_stream_type& os, json_context, T const&)
        { os.start_array(); }
        template < typename T >
        static void
        output(json_stream_type& os, T&& val)
        {
            json_write(os, ::std::forward<T>(val));
        }
        template < typename T >
        static void
        output_value_name(json_stream_type& os, T&& val)
        {
            throw ::std::runtime_error("Cannot use an array as a value key");
        }
        template < typename T >
        static void
        after(json_stream_type& os, json_context, T const&)
        { os.end_array(); }
    };

    template < typename CharT, typename Traits >
    struct value_output_wrapper_impl< CharT, Traits,
            traits::value_type::OBJECT > {
        using json_stream_type = basic_json_ostream<CharT, Traits>;

        template < typename T >
        static void
        before(json_stream_type& os, json_context, T const&)
        { os.start_object(); }
        template < typename T >
        static void
        before(json_stream_type& os, json_context, T* val)
        { if (val)  os.start_object(); }
        template < typename T >
        static void
        output(json_stream_type& os, T&& val)
        {
            json_write(os, ::std::forward<T>(val));
        }
        template < typename T >
        static void
        output(json_stream_type& os, T* val)
        {
            if (val) {
                json_write(os, *val);
            } else {
                os.stream() << json_stream_type::null_str;
                if (os.current_context() == json_context::value)
                    os.pop_context();
            }
        }
        template < typename T >
        static void
        output_value_name(json_stream_type& os, T&& val)
        {
            throw ::std::runtime_error("Cannot use an object as a value key");
        }
        template < typename T >
        static void
        after(json_stream_type& os, json_context, T const&)
        { os.end_object(); }
        template < typename T >
        static void
        after(json_stream_type& os, json_context, T* val)
        {  if (val) os.end_object(); }
    };

}  /* namespace detail */
template < typename CharT, typename Traits, typename T >
struct value_output_wrapper :
    detail::value_output_wrapper_impl< CharT, Traits, traits::json_type<T>::value >{};

template < typename CharT, typename Traits >
class basic_json_ostream : public json_io_base<CharT, Traits> {
public:
    using base_type     = json_io_base<CharT, Traits>;
    using char_type     = typename base_type::char_type;
    using traits_type   = typename base_type::traits_type;
    using this_type     = basic_json_ostream<char_type, traits_type>;
    using stream_type   = typename base_type::ostream_type;
    using chars         = typename base_type::chars;
    using size_type     = ::std::size_t;
    template < typename T >
    using output_wrapper = value_output_wrapper<char_type, traits_type, T>;
public:
    basic_json_ostream(stream_type& os, bool pretty = false)
        : os_{os}, state_{}, pretty_{pretty} {}
    basic_json_ostream(basic_json_ostream const&) = delete;
    basic_json_ostream(basic_json_ostream&& rhs) = default;
    ~basic_json_ostream()
    { flush(); }

    template <typename T>
    this_type&
    write(T&& val)
    {
        using value_type = typename ::std::decay<T>::type;
        using quote_traits = traits::json_quote< value_type >;
        using json_type    = traits::json_type< value_type >;
        using wrapper_type = output_wrapper< value_type >;
        switch (current_context()) {
            case json_context::none:
                if (current_state().second > 0)
                    throw ::std::runtime_error{"Cannot have more than one object at root level"};
                ++current_state().second;
                break;
            case json_context::value:
                // Next is a value in object
                start_value();
                break;
            case json_context::object:
                // Next is a value name in an object
                next_item();
                push_context(json_context::value_key);
                break;
            case json_context::array:
                // This is a value in an array
                next_item();
                break;
            default:
                break;
        }
//        os_ << "C: " << static_cast<int>(current_context()) << " ";
//        os_ << "T: " << static_cast<int>(json_type::value) << " ";
        if (current_context() == json_context::value_key) {
            // Need to quote a value name
            put(chars::double_quote);
            wrapper_type::output_value_name(*this, ::std::forward<T>(val));
            put(chars::double_quote);
            pop_context();
            push_context(json_context::value);
        } else {
            auto ctx = current_context();
            wrapper_type::before(*this, ctx, ::std::forward<T>(val));
            wrapper_type::output(*this, ::std::forward<T>(val));
            wrapper_type::after(*this, ctx, ::std::forward<T>(val));
        }
        return *this;
    }

    this_type&
    start_object(bool ni = false)
    {
        // Check we can start an object here.
        // The only context we cannot do it - object root context
        if (current_context() == json_context::value_key)
            throw ::std::runtime_error{"json start object: invalid state"};
        if (current_context() == json_context::value) {
            start_value();
        } else if (ni) {
            if (current_context() == json_context::none) {
                if (current_state().second > 0)
                    throw ::std::runtime_error{"Cannot have more than one object at root level"};
                ++current_state().second;
            } else {
                next_item();
            }
        }
        push_context(json_context::object);
        put(chars::start_object);
        return *this;
    }
    this_type&
    end_object()
    {
        if (current_context() != json_context::object)
            throw ::std::runtime_error{"json end object: invalid state"};
        bool empty = current_state().second == 0;
        pop_context();
        if (!empty) nl();
        put(chars::end_object);
        return *this;
    }

    this_type&
    start_array(bool ni = false)
    {
        // Check we can start an object here.
        // The only context we cannot do it - object root context
        if (current_context() == json_context::value_key)
            throw ::std::runtime_error{"json start array: invalid state"};
        if (current_context() == json_context::value) {
            start_value();
        } else if (ni) {
            if (current_context() == json_context::none) {
                if (current_state().second > 0)
                    throw ::std::runtime_error{"Cannot have more than one object at root level"};
                ++current_state().second;
            } else {
                next_item();
            }
        }
        push_context(json_context::array);
        put(chars::start_array);
        return *this;
    }
    this_type&
    end_array()
    {
        if (current_context() != json_context::array)
            throw ::std::runtime_error{"json end array: invalid state"};
        bool empty = current_state().second == 0;
        pop_context();
        if (!empty) nl();
        put(chars::end_array);
        return *this;
    }

    this_type&
    apply(void(*manip)(stream_type&))
    {
        manip(os_);
        return *this;
    }

    this_type&
    flush()
    {
        while(!state_.empty()) {
            auto state = state_.top();
            switch (state.first) {
                case json_context::value_key:
                    pop_context();
                    break;
                case json_context::value:
                    write(nullptr);
                    pop_context();
                    break;
                case json_context::object:
                    end_object();
                    break;
                case json_context::array:
                    end_array();
                    break;
                default:
                    break;
            }

        }
        return *this;
    }

    ::std::locale
    getloc() const
    { return os_.getloc(); }
    void
    imbue( ::std::locale const& loc )
    { os_.imbue(loc); }

    stream_type&
    stream()
    { return os_; }
    this_type&
    put(chars c)
    {
        base_type::put(os_, c);
        return *this;
    }
    this_type&
    put(char_type c)
    {
        os_.put(c);
        return *this;
    }

    void
    push_context(json_context ctx)
    { state_.emplace(ctx, size_type{0}); }
    void
    pop_context()
    { if (!state_.empty()) state_.pop(); }
    json_context
    current_context() const
    { return current_state().first; }
private:
    using state_type    = ::std::pair<json_context, size_type>;
    using state_stack   = ::std::stack<::std::pair<json_context, size_type>>;

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

    this_type&
    nl()
    {
        if (pretty_) {
            put(chars::newline);
            if (state_.size() > 0) {
                os_ << ::std::setw(state_.size() * 4)
                    << ::std::setfill(static_cast<char_type>(chars::space))
                    << static_cast<char_type>(chars::space);
            }
        }
        return *this;
    }
    this_type&
    start_value()
    {
        if (pretty_)
            put(chars::space);
        put(chars::colon);
        if (pretty_)
            put(chars::space);
        pop_context();
        return *this;
    }
    this_type&
    next_item()
    {
        if (current_state().second > 0)
            put(chars::comma);
        nl();
        ++current_state().second;
        return *this;
    }
private:
    stream_type&    os_;
    state_type      none_{ json_context::none, 0 };
    state_stack     state_;
    bool            pretty_;
};

extern template class basic_json_ostream<char>;
extern template class basic_json_ostream<wchar_t>;

using json_ostream  = basic_json_ostream<char>;
using wjson_ostream = basic_json_ostream<wchar_t>;

/**
 * Output for basic_json_ostream manipulators
 * @param os
 * @param manip
 * @return
 */
template <typename CharT, typename Traits>
basic_json_ostream<CharT, Traits>&
operator << (basic_json_ostream<CharT, Traits>& os, void(*manip)(basic_json_ostream<CharT, Traits>&))
{
    manip(os);
    return os;
}

/**
 * ::std::ostream manipulator output
 * @param os
 * @param manip
 * @return
 */
template <typename CharT, typename Traits>
basic_json_ostream<CharT, Traits>&
operator << (basic_json_ostream<CharT, Traits>& os,
        void(*manip)(typename basic_json_ostream<CharT, Traits>::stream_type&))
{
    os.apply(manip);
    return os;
}

/**
 * JSON value serialization
 * @param os
 * @param val
 * @return
 */
template < typename CharT, typename Traits, typename T >
basic_json_ostream<CharT, Traits>&
operator << (basic_json_ostream<CharT, Traits>& os, T&& val)
{
    return os.write(::std::forward<T>(val));
}

//----------------------------------------------------------------------------
template < typename CharT, typename Traits >
basic_json_ostream<CharT, Traits>&
json_write(basic_json_ostream<CharT, Traits>& os, CharT const* str)
{
    using chars = typename basic_json_ostream<CharT, Traits>::chars;
    for (auto c = str; static_cast<chars>(*c) != chars::null; ++c) {
        switch (static_cast<chars>(*c)) {
            case chars::double_quote:
                os.put(chars::escape_symbol);
            default:
                os.put(*c);
                break;
        }
    }
    return os;
}

template < typename CharT, typename Traits, typename Allocator >
basic_json_ostream<CharT, Traits>&
json_write(basic_json_ostream<CharT, Traits>& os,
        ::std::basic_string<CharT, Traits, Allocator>&& str)
{
    using chars = typename basic_json_ostream<CharT, Traits>::chars;
    for (auto c : str) {
        switch (static_cast<chars>(c)) {
            case chars::double_quote:
                os.put(chars::escape_symbol);
            default:
                os.put(c);
                break;
        }
    }
    return os;
}

template < typename CharT, typename Traits, typename Allocator >
basic_json_ostream<CharT, Traits>&
json_write(basic_json_ostream<CharT, Traits>& os,
        ::std::basic_string<CharT, Traits, Allocator> const& str)
{
    using chars = typename basic_json_ostream<CharT, Traits>::chars;
    for (auto c : str) {
        switch (static_cast<chars>(c)) {
            case chars::double_quote:
                os.put(chars::escape_symbol);
            default:
                os.put(c);
                break;
        }
    }
    return os;
}

//----------------------------------------------------------------------------
template < typename CharT, typename Traits >
void
start_object(basic_json_ostream<CharT, Traits>& os)
{
    os.start_object(true);
}
template < typename CharT, typename Traits >
void
end_object(basic_json_ostream<CharT, Traits>& os)
{
    os.end_object();
}

template < typename CharT, typename Traits >
void
start_array(basic_json_ostream<CharT, Traits>& os)
{
    os.start_array(true);
}
template < typename CharT, typename Traits >
void
end_array(basic_json_ostream<CharT, Traits>& os)
{
    os.end_array();
}

}  /* namespace json */
}  /* namespace wire */

#include <wire/json/detail/container_output.hpp>

#endif /* WIRE_JSON_JSON_STREAM_HPP_ */
