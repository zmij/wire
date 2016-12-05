/*
 * parser_base.cpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/json/detail/parser_base.hpp>
#include <wire/json/detail/parser_traits.hpp>
#include <wire/json/detail/integral_parser.hpp>
#include <wire/json/detail/floating_parser.hpp>

#include <wire/json/detail/json_fsm.hpp>

namespace afsm {
template class state_machine< wire::json::detail::json_parser_fsm_def<char> >;
template class state_machine< wire::json::detail::json_parser_fsm_def<wchar_t> >;
}  /* namespace afsm */

namespace wire {
namespace json {
namespace detail {

template struct basic_parser_base<char>;
template struct basic_parser_base<wchar_t>;
template struct basic_ignore_parser<char>;
template struct basic_ignore_parser<wchar_t>;
template struct basic_ignore_array_parser<char>;
template struct basic_ignore_array_parser<wchar_t>;
template struct basic_ignore_object_parser<char>;
template struct basic_ignore_object_parser<wchar_t>;
template struct basic_delegate_parser<char>;
template struct basic_delegate_parser<wchar_t>;

template struct basic_integral_parser<char, char>;
template struct basic_integral_parser<unsigned char, char>;
template struct basic_integral_parser<short, char>;
template struct basic_integral_parser<unsigned short, char>;
template struct basic_integral_parser<int, char>;
template struct basic_integral_parser<unsigned int, char>;
template struct basic_integral_parser<long, char>;
template struct basic_integral_parser<unsigned long, char>;
template struct basic_integral_parser<long long, char>;
template struct basic_integral_parser<unsigned long long, char>;

template struct basic_integral_parser<short, wchar_t>;
template struct basic_integral_parser<unsigned short, wchar_t>;
template struct basic_integral_parser<int, wchar_t>;
template struct basic_integral_parser<unsigned int, wchar_t>;
template struct basic_integral_parser<long, wchar_t>;
template struct basic_integral_parser<unsigned long, wchar_t>;
template struct basic_integral_parser<long long, wchar_t>;
template struct basic_integral_parser<unsigned long long, wchar_t>;

template struct basic_boolean_parser<char>;
template struct basic_boolean_parser<wchar_t>;

template struct basic_floating_parser<float, char>;
template struct basic_floating_parser<double, char>;
template struct basic_floating_parser<long double, char>;

template struct basic_floating_parser<float, wchar_t>;
template struct basic_floating_parser<double, wchar_t>;
template struct basic_floating_parser<long double, wchar_t>;

//----------------------------------------------------------------------------
bool
parse(parser_base& p, char const* first, ::std::size_t size)
{
    namespace qi = ::boost::spirit::qi;
    using parser_traits = detail::parser_traits< char const*, char >;
    using json_io = json_io_base<char>;

    parser_traits::tokenizer_type tokens;
    parser_traits::token_iterator iter = tokens.begin(first, first + size);
    parser_traits::token_iterator end = tokens.end();

//    parser_traits::grammar_type grammar(tokens, p);
//    return qi::phrase_parse(iter, end, grammar, qi::in_state(json_io::ws_state)[tokens.self]);
    return false;
}

bool
parse(parser_base& p, ::std::string const& str)
{
    return parse(p, str.data(), str.size());
}

bool
parse(parser_base& p, ::std::istream& is)
{
    namespace qi = ::boost::spirit::qi;
    namespace lex = ::boost::spirit::lex;
    using istream_iterator = ::boost::spirit::istream_iterator;
    using parser_traits = detail::parser_traits< istream_iterator, char >;
    using json_io = json_io_base<char>;
    using tokenizer_type = parser_traits::tokenizer_type;
    using fsm_type = parser_traits::fsm_type;

    is.unsetf(::std::ios_base::skipws);

    istream_iterator sb{is};
    istream_iterator se;

    tokenizer_type tokens;
    parser_traits::token_iterator iter = tokens.begin(sb, se);
    parser_traits::token_iterator end = tokens.end();
    fsm_type fsm;

//    lex::tokenize(iter, end, tokens, ::std::ref(fsm));
    while (iter != end && token_is_valid(*iter)) {
        if (!fsm(*iter))
            return false;
        ++iter;
    }

//    parser_traits::grammar_type grammar(tokens, p);
//    return qi::phrase_parse(iter, end, grammar, qi::in_state(json_io::ws_state)[tokens.self]);
    return fsm.is_in_state< fsm_type::context::end >();
}

bool
parse(wparser_base& p, wchar_t const* first, ::std::size_t size)
{
    namespace qi = ::boost::spirit::qi;
    using parser_traits = detail::parser_traits< wchar_t const*, wchar_t >;
    using json_io = json_io_base<wchar_t>;

    parser_traits::tokenizer_type tokens;
    parser_traits::token_iterator iter = tokens.begin(first, first + size);
    parser_traits::token_iterator end = tokens.end();

//    parser_traits::grammar_type grammar(tokens, p);
//    return qi::phrase_parse(iter, end, grammar, qi::in_state(json_io::ws_state)[tokens.self]);
    return false;
}

bool
parse(wparser_base& p, ::std::wstring const& str)
{
    return parse(p, str.data(), str.size());
}

bool
parse(wparser_base& p, ::std::wistream& is)
{
    namespace qi = ::boost::spirit::qi;
    using istream_iterator = ::boost::spirit::basic_istream_iterator<wchar_t>;
    using parser_traits = detail::parser_traits< istream_iterator, wchar_t >;
    using json_io = json_io_base<wchar_t>;

    istream_iterator sb{is};
    istream_iterator se;

    parser_traits::tokenizer_type tokens;
    parser_traits::token_iterator iter = tokens.begin(sb, se);
    parser_traits::token_iterator end = tokens.end();

//    parser_traits::grammar_type grammar(tokens, p);
//    return qi::phrase_parse(iter, end, grammar, qi::in_state(json_io::ws_state)[tokens.self]);
    return false;
}

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */
