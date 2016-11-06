/*
 * json_io_base.hpp
 *
 *  Created on: 5 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_JSON_IO_BASE_HPP_
#define WIRE_JSON_JSON_IO_BASE_HPP_

#include <string>
#include <iosfwd>

namespace wire {
namespace json {

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct json_io_base {
    using char_type     = CharT;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    using string_type   = ::std::basic_string<char_type, traits_type>;
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
    static constexpr char_type const* true_str          = nullptr;
    static constexpr char_type const* false_str         = nullptr;
    static constexpr char_type const* null_str          = nullptr;

    static constexpr char_type const* string_re         = nullptr;
    static constexpr char_type const* empty_re          = nullptr;
    static constexpr char_type const* integral_re       = nullptr;
    static constexpr char_type const* float_re          = nullptr;
    static constexpr char_type const* ws_re             = nullptr;
    static constexpr char_type const* c_comment_re      = nullptr;
    static constexpr char_type const* cpp_comment_re    = nullptr;

    static constexpr char_type const* ws_state          = nullptr;

    static ostream_type&
    put(ostream_type& os, chars);
    static char_type
    cvt(chars);
    template < typename T >
    static string_type
    to_string(T&&);
};

template < typename Traits >
struct json_io_base<char, Traits> {
    using char_type     = char;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    using string_type   = ::std::basic_string<char_type, traits_type>;
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
    static constexpr char_type const* true_str          = "true";
    static constexpr char_type const* false_str         = "false";
    static constexpr char_type const* null_str          = "null";

    static constexpr char_type const* string_re         = R"~(\"((\\\")|(\\.)|[^\"])+\")~";
    static constexpr char_type const* empty_re          = R"~(\"\")~";
    static constexpr char_type const* integral_re       = "-?([1-9][0-9]*)|0";
    static constexpr char_type const* float_re          = "-?([1-9][0-9]*)?\\.[0-9]*([eE]-?[1-9][0-9]*)?";
    static constexpr char_type const* ws_re             = "[ \\t\\n]+";
    static constexpr char_type const* c_comment_re      = R"~(\/\*[^*]*\*+([^/*][^*]*\*+)*\/)~";
    static constexpr char_type const* cpp_comment_re    = R"~(\/\/.*?\n)~";

    static constexpr char_type const* ws_state          = "WS";

    static ostream_type&
    put(ostream_type& os, chars v)
    {
        os.put(static_cast<char_type>(v));
        return os;
    }
    static char_type
    cvt(chars v)
    { return static_cast<char_type>(v); }

    template < typename T >
    static string_type
    to_string(T&& v)
    {
        return ::std::to_string(::std::forward<T>(v));
    }
};

template < typename Traits >
constexpr char const*
json_io_base<char, Traits>::c_comment_re;
template < typename Traits >
constexpr char const*
json_io_base<char, Traits>::cpp_comment_re;
template < typename Traits >
constexpr char const*
json_io_base<char, Traits>::ws_state;

template < typename Traits >
struct json_io_base<wchar_t, Traits> {
    using char_type     = wchar_t;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    using string_type   = ::std::basic_string<char_type, traits_type>;
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
    static constexpr char_type const* true_str          = L"true";
    static constexpr char_type const* false_str         = L"false";
    static constexpr char_type const* null_str          = L"null";

    static constexpr char_type const* string_re         = LR"~(\"((\\\")|(\\.)|[^\"])+\")~";
    static constexpr char_type const* empty_re          = LR"~(\"\")~";
    static constexpr char_type const* integral_re       = L"-?([1-9][0-9]*)|0";
    static constexpr char_type const* float_re          = L"-?([1-9][0-9]*)?\\.[0-9]*([eE]-?[1-9][0-9]*)?";
    static constexpr char_type const* ws_re             = L"[ \\t\\n]+";
    static constexpr char_type const* c_comment_re      = LR"~(\/\*[^*]*\*+([^/*][^*]*\*+)*\/)~";
    static constexpr char_type const* cpp_comment_re    = LR"~(\/\/.*?\n)~";

    static constexpr char_type const* ws_state          = L"WS";

    static ostream_type&
    put(ostream_type& os, chars v)
    {
        os.put(static_cast<char_type>(v));
        return os;
    }
    constexpr static char_type
    cvt(chars v)
    { return static_cast<char_type>(v); }

    template < typename T >
    static string_type
    to_string(T&& v)
    {
        return ::std::to_wstring(::std::forward<T>(v));
    }
};

template < typename Traits >
constexpr wchar_t const*
json_io_base<wchar_t, Traits>::c_comment_re;
template < typename Traits >
constexpr wchar_t const*
json_io_base<wchar_t, Traits>::cpp_comment_re;
template < typename Traits >
constexpr wchar_t const*
json_io_base<wchar_t, Traits>::ws_state;

enum class json_context : ::std::uint8_t {
    none,       /**< JSON output hasn't been started */
    value_key,  /**< A value key is required here (object scope) */
    value,      /**< A value is required here (after a value key, object scope) */
    object,     /**< A value key is required here (object scope) */
    array,      /**< A value is required here (array scope) */
};

}  /* namespace json */
}  /* namespace wire */



#endif /* WIRE_JSON_JSON_IO_BASE_HPP_ */
