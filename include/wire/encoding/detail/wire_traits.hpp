/*
 * wire_traits.hpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#ifndef WIRE_DETAIL_WIRE_TRAITS_HPP_
#define WIRE_DETAIL_WIRE_TRAITS_HPP_

#include <wire/encoding/types.hpp>

#include <string>
#include <vector>
#include <list>
#include <array>
#include <set>
#include <unordered_set>
#include <queue>
#include <deque>
#include <map>
#include <unordered_map>

namespace wire {
namespace encoding {
namespace detail {

enum wire_types {
	SCALAR_FIXED,
	SCALAR_VARINT,
	SCALAR_WITH_SIZE,
	ARRAY_FIXED,
	ARRAY_VARLEN,
	DICTIONARY,
	STRUCT,
	CLASS
};

template < typename T, bool >
struct wire_polymorphic_type;

template < typename T >
struct wire_polymorphic_type< T, true > : std::integral_constant< wire_types, CLASS > {};

template < typename T >
struct wire_polymorphic_type< T, false > : std::integral_constant< wire_types, STRUCT > {};

template < typename T, bool >
struct wire_enum_type;

template < typename T >
struct wire_enum_type< T, true > : std::integral_constant< wire_types, SCALAR_VARINT > {};
template < typename T >
struct wire_enum_type< T, false > : wire_polymorphic_type< T, std::is_polymorphic<T>::value  >{};

template < typename T >
struct wire_type : wire_enum_type< T, std::is_enum<T>::value > {};

//@{
/** @name Fixed wire size types */
template <>
struct wire_type<bool> : std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type<char> : std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type<int8_t> : std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type<uint8_t> : std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type< float > : std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type< double > : std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type< long double > : std::integral_constant< wire_types, SCALAR_FIXED > {};

template < typename T >
struct wire_type< fixed_size< T > > : std::integral_constant< wire_types, SCALAR_FIXED > {};
//@}

template <>
struct wire_type< int16_t > : std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< int32_t > : std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< int64_t > : std::integral_constant< wire_types, SCALAR_VARINT > {};

template <>
struct wire_type< uint16_t > : std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< uint32_t > : std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< uint64_t > : std::integral_constant< wire_types, SCALAR_VARINT > {};

template <>
struct wire_type< std::string > : std::integral_constant< wire_types, SCALAR_WITH_SIZE > {};

template < typename T, std::size_t N >
struct wire_type< std::array< T, N > > : std::integral_constant< wire_types, ARRAY_FIXED > {};

template < typename T >
struct wire_type< std::vector< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T >
struct wire_type< std::list< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T >
struct wire_type< std::queue< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T >
struct wire_type< std::deque< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T >
struct wire_type< std::set< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T >
struct wire_type< std::multiset< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T >
struct wire_type< std::unordered_set< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T >
struct wire_type< std::unordered_multiset< T > > : std::integral_constant< wire_types, ARRAY_VARLEN > {};

template < typename K, typename V >
struct wire_type< std::map< K, V > > : std::integral_constant< wire_types, DICTIONARY > {};
template < typename K, typename V >
struct wire_type< std::multimap< K, V > > : std::integral_constant< wire_types, DICTIONARY > {};
template < typename K, typename V >
struct wire_type< std::unordered_map< K, V > > : std::integral_constant< wire_types, DICTIONARY > {};
template < typename K, typename V >
struct wire_type< std::unordered_multimap< K, V > > : std::integral_constant< wire_types, DICTIONARY > {};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_DETAIL_WIRE_TRAITS_HPP_ */
