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
#include <sstream>
#include <locale>
#include <codecvt>
#include <algorithm>

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
    using istringstream = ::std::basic_istringstream<char_type, traits_type>;
    enum class chars {
        null,
        double_quote,
        escape_symbol,
        slash,
        start_object,
        end_object,
        start_array,
        end_array,
        colon,
        comma,
        space,
        // Escaped symbols
        newline,
        carriage_ret,
        backspace,
        tab,
        form_feed,
        // Hex digits
        zero,
        nine,
        a,
        f,
        A,
        F,
    };
    enum class escaped {
        double_quote,
        escape_symbol,
        slash,
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

    static bool
    is_cntrl(char_type c);

    static char_type
    get_escaped(char_type c);
    static char_type
    get_unescaped(char_type c);

    static void
    add_codepoint(string_type& str, char32_t c);
};

template < typename Traits >
struct json_io_base<char, Traits> : json_token_defs {
    using char_type     = char;
    using traits_type   = Traits;
    using ostream_type  = ::std::basic_ostream<char_type, traits_type>;
    using istream_type  = ::std::basic_istream<char_type, traits_type>;
    using string_type   = ::std::basic_string<char_type, traits_type>;
    using istringstream = ::std::basic_istringstream<char_type, traits_type>;

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
        // Escaped symbols
        newline         = '\n',
        carriage_ret    = '\r',
        backspace       = '\b',
        tab             = '\t',
        form_feed       = '\f',
        // Hex digits
        zero            = '0',
        nine            = '9',
        a               = 'a',
        f               = 'f',
        A               = 'A',
        F               = 'F',
    };
    enum class escaped : char_type {
        double_quote    = '"',
        escape_symbol   = '\\',
        slash           = '/',
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
    static constexpr char_type const* integral_re       = "-?(([1-9][0-9]*)|0)";
    static constexpr char_type const* float_re          =
            "-?(([1-9][0-9]*)|0)"               // Integral part
            "(\\.[0-9]+)?"                      // Optional fractional part
            "([Ee](\\+|-)?[0-9]+)?"             // Optional exponent
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

    static bool
    is_cntrl(char_type c)
    {
        if (c == '\177') // Del character
            return false;
        return ::std::iscntrl(c, ::std::locale{});
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
            case escaped::double_quote:
            case escaped::escape_symbol:
            case escaped::slash:
                return c;
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

    static void
    add_codepoint(string_type& str, char32_t c)
    {
        ::std::wstring_convert<::std::codecvt_utf8<char32_t>, char32_t> converter;
        string_type tmp = converter.to_bytes(c);
        ::std::copy(tmp.begin(), tmp.end(), ::std::back_inserter(str));
    }

    template < typename Iterator >
    static integral_type
    extract_integer(Iterator begin, Iterator end)
    {
        istringstream is(string_type{begin, end});
        integral_type res;
        if (!(is >> res))
            // FIXME Throw an exception here
            return res;
        return res;
    }

    template < typename Iterator >
    static float_type
    extract_float(Iterator begin, Iterator end)
    {
        istringstream is(string_type{begin, end});
        float_type res;
        if (!(is >> res))
            // FIXME Throw an exception here
            return res;
        return res;
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
    using istringstream = ::std::basic_istringstream<char_type, traits_type>;

    enum class chars : char_type {
        null            = 0,
        double_quote    = L'"',
        escape_symbol   = L'\\',
        slash           = L'/',
        start_object    = L'{',
        end_object      = L'}',
        start_array     = L'[',
        end_array       = L']',
        colon           = L':',
        comma           = L',',
        space           = L' ',
        // Escaped symbols
        newline         = L'\n',
        carriage_ret    = L'\r',
        backspace       = L'\b',
        tab             = L'\t',
        form_feed       = L'\f',
        // Hex digits
        zero            = L'0',
        nine            = L'9',
        a               = L'a',
        f               = L'f',
        A               = L'A',
        F               = L'F',
    };
    enum class escaped : char_type {
        double_quote    = L'"',
        escape_symbol   = L'\\',
        slash           = L'/',
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
    static constexpr char_type const* integral_re       = L"-?(([1-9][0-9]*)|0)";
    static constexpr char_type const* float_re          =
            L"-?(([1-9][0-9]*)|0)"              // Integral part
            L"(\\.[0-9]+)?"                     // Optional fractional part
            L"([Ee](\\+|-)?[0-9]+)?"            // Optional exponent
            ;
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

    static bool
    is_cntrl(char_type c)
    {
        return ::std::iscntrl(c, ::std::locale{});
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
            case escaped::double_quote:
            case escaped::escape_symbol:
            case escaped::slash:
                return c;
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

    static void
    add_codepoint(string_type& str, char32_t c)
    {
        // FIXME Implement adding wchar_t codepoint
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

/**
 *
 * @param begin
 * @param end
 * @return A pair of value and bool. If the flat is true, the value was parsed
 *         and contains integral codepoint. Otherwise it contains number of
 *         bytes read for error reporting.
 */
template < typename InputIterator >
::std::pair<char32_t, bool>
unicode_escape_sequence(InputIterator& begin, InputIterator end)
{
    using iterator_traits   = ::std::iterator_traits<InputIterator>;
    using char_type         = typename iterator_traits::value_type;
    using json_io           = json_io_base<char_type>;
    using chars             = typename json_io::chars;

    char32_t res{0};
    int i = 0;
    for (; i < 4 && begin != end; ++i) {
        res <<= 4;
        char_type c = *begin++;
        if (c >= json_io::cvt(chars::zero) && c <= json_io::cvt(chars::nine)) {
            res += c - json_io::cvt(chars::zero);
        } else if (c >= json_io::cvt(chars::a) && c <= json_io::cvt(chars::f)) {
            res += c - json_io::cvt(chars::a) + 10;
        } else if (c >= json_io::cvt(chars::A) && c <= json_io::cvt(chars::F)) {
            res += c - json_io::cvt(chars::A) + 10;
        } else {
            break;
        }
    }
    if (i < 4) {
        return {i, false};
    }
    return {res, true};
}

template <typename InputIterator>
::std::pair<char32_t, bool>
unicode_codepoint(InputIterator& begin, InputIterator end)
{
    using iterator_traits   = ::std::iterator_traits<InputIterator>;
    using char_type         = typename iterator_traits::value_type;
    using json_io           = json_io_base<char_type>;
    using chars             = typename json_io::chars;
    using escaped           = typename json_io::escaped;

    auto res = unicode_escape_sequence(begin, end);
    if (!res.second)
        return res;

    if (0xd800 <= res.first && res.first <= 0xdbff) {
        // Surrogate pair
        if (begin == end)
            return {4, false};
        if (*begin++ != json_io::cvt(chars::escape_symbol) || begin == end)
            return {5, false};
        if (*begin++ != json_io::cvt(escaped::utf_codepoint) || begin == end)
            return {6, false};
        auto scnd = unicode_escape_sequence(begin, end);
        if (!scnd.second) {
            scnd.first += 6;
            return scnd;
        }
        res.first = 0x10000 + ((res.first & 0x3ff) << 10) + (scnd.first & 0x3ff);
    }

    return res;
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
    // Skip opening quote
    ++begin;
    for (::std::size_t dist = 0; begin != end; ++dist) {
        auto c = *begin;
        if (c == json_io::cvt(chars::escape_symbol)) {
            // Get next symbol
            ++begin;
            if (begin != end) {
                ++dist;
                c = *begin;
                if (c == json_io::cvt(escaped::utf_codepoint)) {
                    // Read unicode escape sequence
                    ++begin;
                    ++dist;
                    auto codepoint = unicode_codepoint(begin, end);
                    if (!codepoint.second) {
                        dist += codepoint.first;
                        // FIXME Throw an exception here with `dist` as position
                        return false;
                    }
                    dist += (codepoint.first >= 0x10000) ? 10 : 4;
                    // Convert the character to current char type
                    json_io::add_codepoint(str, codepoint.first);
                } else {
                    c = json_io::get_unescaped(c);
                    if (c == json_io::cvt(chars::null)) { // Unknown escape character
                        // FIXME Throw an exception here with `dist` as position
                        return false;
                    }
                    ++begin;
                    if (begin != end) // Not the closing quote
                        str.push_back(c);
                    else
                        // FIXME Throw an exception here with `dist` as position
                        // dangling escape
                        return false;
                }
            } else {
                // FIXME Throw an exception here with `dist` as position
                return false;
            }
        } else {
            // Check for unescaped symbol
            if (json_io::is_cntrl(c))
                // FIXME Throw an exception here with `dist` as position
                return false;
            ++begin;
            if (begin != end) // Not the closing quote
                str.push_back(c);
        }
    }
    return true;
}


}  /* namespace json */
}  /* namespace wire */



#endif /* WIRE_JSON_JSON_IO_BASE_HPP_ */
