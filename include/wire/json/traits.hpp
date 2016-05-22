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
    STRING,
    NUMBER,
    OBJECT,
    ARRAY,
    BOOL
};

template < value_type V >
using json_type_constant = ::std::integral_constant< value_type, V >;

template < typename T >
struct json_type : util::conditional_constant<
    value_type,
    ::std::is_enum<T>::value,
         value_type::NUMBER,
         util::conditional_constant<value_type,
            util::has_iostream_operators< T >::value,
            value_type::STRING, value_type::OBJECT>::value > {};

//@{
/** @name Integral types */
template <>
struct json_type<bool> : json_type_constant< value_type::BOOL >{};
template<>
struct json_type<char> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<signed char> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<unsigned char> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<short> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<unsigned short> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<int> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<unsigned int> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<long> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<unsigned long> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<long long> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<unsigned long long> : json_type_constant< value_type::NUMBER >{};
//@}
//@{
/** @name Floating-point types */
template<>
struct json_type<float> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<double> : json_type_constant< value_type::NUMBER >{};
template<>
struct json_type<long double> : json_type_constant< value_type::NUMBER >{};
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
struct json_type< ::std::unordered_multimap< T, Rest ... > > : json_type_constant< value_type::ARRAY >{};

template < typename K, typename V, typename ... Rest >
struct json_type< ::std::map< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::multimap< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::unordered_map< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::unordered_multimap< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
//@}

}  /* namespace traits */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_TRAITS_HPP_ */
