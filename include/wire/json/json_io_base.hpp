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

struct json_token_defs {
    enum token_ids {
        id_string        = 1000,
        id_empty_string,
        id_integral,
        id_float,
        id_true,
        id_false,
        id_null,
        id_newline,
        id_ws
    };
    using integral_type     = ::std::int64_t;
    using float_type        = long double;
};

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct json_io_base : json_token_defs {
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
        carriage_ret,
        backspace,
        tab,
        form_feed,
    };
    enum class escaped {
        newline,
        carriage_ret,
        backspace,
        tab,
        form_feed,
        utf_codepoint,
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
    static constexpr char_type
    cvt(chars);
    static constexpr char_type
    cvt(escaped);
    template < typename T >
    static string_type
    to_string(T&&);

    static char_type
    get_escaped(char_type c);
    static char_type
    get_unescaped(char_type c);
};

template < typename Traits >
struct json_io_base<char, Traits> : json_token_defs {
    using char_type     = char;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    using string_type   = ::std::basic_string<char_type, traits_type>;

    enum class chars : char_type {
        null            = 0,
        double_quote    = '"',
        escape_symbol   = '\\',
        slash           = '/',
        start_object    = '{',
        end_object      = '}',
        start_array     = '[',
        end_array       = ']',
        colon           = ':',
        comma           = ',',
        space           = ' ',
        newline         = '\n',
        carriage_ret    = '\r',
        backspace       = '\b',
        tab             = '\t',
        form_feed       = '\f',
    };
    enum class escaped : char_type {
        newline         = 'n',
        carriage_ret    = 'r',
        backspace       = 'b',
        tab             = 't',
        form_feed       = 'f',
        utf_codepoint   = 'u',
    };
    static constexpr char_type const* true_str          = "true";
    static constexpr char_type const* false_str         = "false";
    static constexpr char_type const* null_str          = "null";

    static constexpr char_type const* string_re         = R"~(\"((\\\")|(\\.)|[^\"])+\")~";
    static constexpr char_type const* empty_re          = R"~(\"\")~";
    static constexpr char_type const* integral_re       = "(-?[1-9][0-9]*)|0";
    static constexpr char_type const* float_re          =
            "-?"    // Optional minus
            "(((([1-9][0-9]*)|0)(\\.[0-9]*)?)"  // Integral part with optional fractional part
            "|(\\.[0-9]*))"                     // Or fractional part only
            "([eE][+-]?[1-9][0-9]*)?"           // Optional exponent
            ;
    static constexpr char_type const* ws_re             = "[ \\t]+";
    static constexpr char_type const* c_comment_re      = R"~(\/\*[^*]*\*+([^/*][^*]*\*+)*\/)~";
    static constexpr char_type const* cpp_comment_re    = R"~(\/\/.*?\n)~";

    static constexpr char_type const* ws_state          = "WS";

    static ostream_type&
    put(ostream_type& os, chars v)
    {
        os.put(static_cast<char_type>(v));
        return os;
    }
    static constexpr char_type
    cvt(chars v)
    { return static_cast<char_type>(v); }
    static constexpr char_type
    cvt(escaped v)
    { return static_cast<char_type>(v); }

    template < typename T >
    static string_type
    to_string(T&& v)
    {
        return ::std::to_string(::std::forward<T>(v));
    }

    static char_type
    get_escaped(char_type c)
    {
        switch (static_cast<chars>(c)) {
            case chars::newline:
                return cvt(escaped::newline);
            case chars::carriage_ret:
                return cvt(escaped::carriage_ret);
            case chars::backspace:
                return cvt(escaped::backspace);
            case chars::tab:
                return cvt(escaped::tab);
            case chars::form_feed:
                return cvt(escaped::form_feed);
            default:
                break;
        }
        return cvt(chars::null);
    }
    static char_type
    get_unescaped(char_type c)
    {
        switch(static_cast<escaped>(c)) {
            case escaped::newline:
                return cvt(chars::newline);
            case escaped::carriage_ret:
                return cvt(chars::carriage_ret);
            case escaped::backspace:
                return cvt(chars::backspace);
            case escaped::tab:
                return cvt(chars::tab);
            case escaped::form_feed:
                return cvt(chars::form_feed);
            default:
                break;
        }
        return cvt(chars::null);
    }

    template < typename Iterator >
    static integral_type
    extract_integer(Iterator begin, Iterator end)
    {
        return integral_type{};
    }

    template < typename Iterator >
    static float_type
    extract_float(Iterator begin, Iterator end)
    {
        return float_type{};
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
struct json_io_base<wchar_t, Traits> : json_token_defs {
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
        carriage_ret    = L'\r',
        backspace       = L'\b',
        tab             = L'\t',
        form_feed       = L'\f',
    };
    enum class escaped : char_type {
        newline         = L'n',
        carriage_ret    = L'r',
        backspace       = L'b',
        tab             = L't',
        form_feed       = L'f',
        utf_codepoint   = L'u',
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
    static constexpr char_type
    cvt(chars v)
    { return static_cast<char_type>(v); }
    static constexpr char_type
    cvt(escaped v)
    { return static_cast<char_type>(v); }

    template < typename T >
    static string_type
    to_string(T&& v)
    {
        return ::std::to_wstring(::std::forward<T>(v));
    }

    static char_type
    get_escaped(char_type c)
    {
        switch (static_cast<chars>(c)) {
            case chars::newline:
                return cvt(escaped::newline);
            case chars::carriage_ret:
                return cvt(escaped::carriage_ret);
            case chars::backspace:
                return cvt(escaped::backspace);
            case chars::tab:
                return cvt(escaped::tab);
            case chars::form_feed:
                return cvt(escaped::form_feed);
            default:
                break;
        }
        return cvt(chars::null);
    }
    static char_type
    get_unescaped(char_type c)
    {
        switch(static_cast<escaped>(c)) {
            case escaped::newline:
                return cvt(chars::newline);
            case escaped::carriage_ret:
                return cvt(chars::carriage_ret);
            case escaped::backspace:
                return cvt(chars::backspace);
            case escaped::tab:
                return cvt(chars::tab);
            case escaped::form_feed:
                return cvt(chars::form_feed);
            default:
                break;
        }
        return cvt(chars::null);
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


template < typename Stream, typename CharT, typename Traits = ::std::char_traits<CharT> >
Stream&
escape(Stream& os, CharT const* str)
{
    using json_io = json_io_base<CharT, Traits>;
    using chars   = typename json_io::chars;

    for (auto c = str; *c != json_io::cvt(chars::null); ++c) {
        switch (static_cast<chars>(*c)) {
            case chars::newline:
            case chars::carriage_ret:
            case chars::backspace:
            case chars::tab:
            case chars::form_feed:
                os.put(chars::escape_symbol);
                os.put(json_io::get_escaped(*c));
                break;
            case chars::double_quote:
            case chars::escape_symbol:
            case chars::slash:
                os.put(chars::escape_symbol);
            default:
                os.put(*c);
                break;
        }
    }
    return os;
}

template < typename InputIterator >
char32_t
read_codepoint(InputIterator begin, InputIterator end)
{
    char32_t res;
}

template < typename InputIterator, typename CharT, typename Traits, typename Allocator >
bool
unescape(InputIterator begin,
        InputIterator end, ::std::basic_string<CharT, Traits, Allocator>& str)
{
    using json_io = json_io_base<CharT, Traits>;
    using chars   = typename json_io::chars;
    using escaped = typename json_io::escaped;

    str.clear();
    str.reserve( ::std::distance(begin, end) );
    // TODO Cut the quotes
    for (::std::size_t dist = 0; begin != end; ++begin, ++dist) {
        auto c = *begin;
        if (c == json_io::cvt(chars::escape_symbol)) {
            // Get next symbol
            ++begin;
            if (begin != end) {
                ++dist;
                c = *begin;
                if (c == json_io::cvt(escaped::utf_codepoint)) {
                    // Read next four symbols
                } else {
                    c = json_io::get_unescaped(c);
                    if (c == json_io::cvt(chars::null)) { // Unknown escape character
                        // FIXME Throw an exception here with `dist` as position
                        return false;
                    }
                    str.push_back(c);
                }
            } else {
                // FIXME Throw an exception here with `dist` as position
                return false;
            }
        } else {
            str.push_back(c);
        }
    }
    return true;
}

}  /* namespace json */
}  /* namespace wire */



#endif /* WIRE_JSON_JSON_IO_BASE_HPP_ */
