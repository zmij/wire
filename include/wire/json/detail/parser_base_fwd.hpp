/*
 * parser_base_fwd.hpp
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_PARSER_BASE_FWD_HPP_
#define WIRE_JSON_DETAIL_PARSER_BASE_FWD_HPP_

namespace wire {
namespace json {

template < typename T, typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_parser;

template < typename T >
using parser        = basic_parser<T, char>;
template < typename T >
using wparser       = basic_parser<T, wchar_t>;

namespace detail {

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_parser_base;
template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_ignore_parser;
template <typename CharT, typename Traits = ::std::char_traits<CharT>>
struct basic_ignore_object_parser;
template <typename CharT, typename Traits = ::std::char_traits<CharT>>
struct basic_ignore_array_parser;
template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_delegate_parser;

using parser_base       = basic_parser_base<char>;
using wparser_base      = basic_parser_base<wchar_t>;

using ignore_parser     = basic_ignore_parser<char>;
using wignore_parser    = basic_ignore_parser<wchar_t>;

using delegate_parser   = basic_delegate_parser<char>;
using wdelegate_parser  = basic_delegate_parser<wchar_t>;

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */



#endif /* WIRE_JSON_DETAIL_PARSER_BASE_FWD_HPP_ */
