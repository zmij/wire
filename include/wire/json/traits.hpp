/*
 * traits.hpp
 *
 *  Created on: 22 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_TRAITS_HPP_
#define WIRE_JSON_TRAITS_HPP_

#include <wire/util/has_io_operator.hpp>
#include <wire/util/integral_constants.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <list>
#include <array>
#include <set>
#include <unordered_set>
#include <queue>
#include <deque>
#include <map>
#include <unordered_map>

#include <memory>

namespace wire {
namespace json {
namespace traits {

enum class value_type {
    VOID,
    BOOL,
    INTEGRAL,
    FLOATING,
    STRING,
    ARRAY,
    OBJECT,
};

template < value_type V >
using json_type_constant = ::std::integral_constant< value_type, V >;
template < bool Condition, value_type IfTrue, value_type IfFalse >
using conditional_tag
        = util::conditional_constant< value_type, Condition, IfTrue, IfFalse >;

template < typename T >
struct json_type : conditional_tag<
    ::std::is_enum<T>::value,                           /*if*/
        value_type::INTEGRAL,
        conditional_tag<
            ::std::is_fundamental< T >::value,          /* if */
            conditional_tag<
                ::std::is_integral< T >::value,         /* if */
                conditional_tag<
                    ::std::is_same< T, bool >::value,   /* if */
                    value_type::BOOL,
                    value_type::INTEGRAL
                >::value,
                conditional_tag<
                    ::std::is_floating_point<T>::value, /* if */
                    value_type::FLOATING,
                    value_type::VOID
                >::value
            >::value,
            conditional_tag<
                util::has_iostream_operators< T >::value,   /* if */
                value_type::STRING,
                value_type::OBJECT>::value
         >::value > {};

//@{
/** @name Pointer types */
template < typename T >
struct json_type<T*> : json_type<T> {};
template < typename T >
struct json_type<T&> : json_type<T> {};
template < typename T >
struct json_type< ::std::shared_ptr<T> > : json_type<T> {};
template < typename T >
struct json_type< ::std::unique_ptr<T> > : json_type<T> {};

template <typename T, ::std::size_t N>
struct json_type< T[N] > : json_type_constant< value_type::ARRAY > {};

template <>
struct json_type<char*> : json_type_constant< value_type::STRING > {};
template <>
struct json_type<char const*> : json_type_constant< value_type::STRING > {};
//@}

//@{
/** @name Container types */
template < typename T, typename ... Rest>
struct json_type< ::std::vector< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};
template < typename T, typename ... Rest>
struct json_type< ::std::list< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};
template < typename T, typename ... Rest>
struct json_type< ::std::deque< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};
template < typename T, typename ... Rest>
struct json_type< ::std::set< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};
template < typename T, typename ... Rest>
struct json_type< ::std::multiset< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};
template < typename T, typename ... Rest>
struct json_type< ::std::unordered_set< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};
template < typename T, typename ... Rest>
struct json_type< ::std::unordered_multiset< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::map< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::multimap< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::unordered_map< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::unordered_multimap< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
//@}

template <typename T>
struct json_quote : ::std::conditional<
        json_type<T>::value == value_type::STRING,
        ::std::true_type,
        ::std::false_type
    >::type {};

}  /* namespace traits */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_TRAITS_HPP_ */
