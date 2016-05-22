/*
 * wire_traits.hpp
 *
 *  Created on: Dec 11, 2015
 *      Author: zmij
 */

#ifndef WIRE_DETAIL_WIRE_TRAITS_HPP_
#define WIRE_DETAIL_WIRE_TRAITS_HPP_

#include <wire/encoding/types.hpp>
#include <wire/util/integral_constants.hpp>

#include <type_traits>
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
#include <memory>

namespace wire {
namespace encoding {
namespace detail {

enum wire_types {
    SCALAR_FIXED,
    SCALAR_VARINT,
    SCALAR_WITH_SIZE,
    VARIANT,
    ARRAY_FIXED,
    ARRAY_VARLEN,
    DICTIONARY,
    STRUCT,
    CLASS,
    EXCEPTION,
    PROXY
};

template < wire_types V >
using wire_type_constant = ::std::integral_constant< wire_types, V >;

template < typename T >
struct is_user_exception : ::std::false_type{};
template < typename T >
struct is_proxy : ::std::false_type{};

template < typename T, bool >
struct wire_polymorphic_type;

template < typename T >
struct wire_polymorphic_type< T, true >
    : util::conditional_constant<wire_types,
            is_user_exception<T>::value, EXCEPTION,
            util::conditional_constant< wire_types,
                is_proxy<T>::value, PROXY, CLASS>::value
    > {};

template < typename T >
struct wire_polymorphic_type< T, false > : ::std::integral_constant< wire_types, STRUCT > {};

template < typename T, bool >
struct wire_enum_type;

template < typename T >
struct wire_enum_type< T, true > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};
template < typename T >
struct wire_enum_type< T, false > : wire_polymorphic_type< T, ::std::is_polymorphic<T>::value  >{};

template < typename T >
struct wire_type : wire_enum_type< T, ::std::is_enum<T>::value > {};

//@{
/** @name Fixed wire size types */
template <>
struct wire_type<bool> : ::std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type<char> : ::std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type<int8_t> : ::std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type<uint8_t> : ::std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type< float > : ::std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type< double > : ::std::integral_constant< wire_types, SCALAR_FIXED > {};
template <>
struct wire_type< long double > : ::std::integral_constant< wire_types, SCALAR_FIXED > {};

template < typename T >
struct wire_type< fixed_size< T > > : ::std::integral_constant< wire_types, SCALAR_FIXED > {};
//@}

template <>
struct wire_type< int16_t > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< int32_t > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< long > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< long long > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};


template <>
struct wire_type< uint16_t > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< uint32_t > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< unsigned long > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};
template <>
struct wire_type< unsigned long long > : ::std::integral_constant< wire_types, SCALAR_VARINT > {};

template <>
struct wire_type< ::std::string > : ::std::integral_constant< wire_types, SCALAR_WITH_SIZE > {};

template < typename T, ::std::size_t N >
struct wire_type< ::std::array< T, N > > : ::std::integral_constant< wire_types, ARRAY_FIXED > {};

template < typename T, typename ... Rest >
struct wire_type< ::std::vector< T, Rest ... > > : ::std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T, typename ... Rest >
struct wire_type< ::std::list< T, Rest ... > > : ::std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T, typename ... Rest >
struct wire_type< ::std::deque< T, Rest ... > > : ::std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T, typename ... Rest >
struct wire_type< ::std::set< T, Rest ... > > : ::std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T, typename ... Rest >
struct wire_type< ::std::multiset< T, Rest ... > > : ::std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T, typename ... Rest >
struct wire_type< ::std::unordered_set< T, Rest ... > > : ::std::integral_constant< wire_types, ARRAY_VARLEN > {};
template < typename T, typename ... Rest >
struct wire_type< ::std::unordered_multiset< T, Rest ... > > : ::std::integral_constant< wire_types, ARRAY_VARLEN > {};

template < typename K, typename V, typename ... Rest >
struct wire_type< ::std::map< K, V, Rest ... > > : ::std::integral_constant< wire_types, DICTIONARY > {};
template < typename K, typename V, typename ... Rest >
struct wire_type< ::std::multimap< K, V, Rest ... > > : ::std::integral_constant< wire_types, DICTIONARY > {};
template < typename K, typename V, typename ... Rest >
struct wire_type< ::std::unordered_map< K, V, Rest ... > > : ::std::integral_constant< wire_types, DICTIONARY > {};
template < typename K, typename V, typename ... Rest >
struct wire_type< ::std::unordered_multimap< K, V, Rest ... > > : ::std::integral_constant< wire_types, DICTIONARY > {};

template < typename T >
struct polymorphic_type {
    using type = typename ::std::decay<T>::type;
};

template < typename T >
struct polymorphic_type< ::std::shared_ptr< T > > : polymorphic_type< T >{};

template < typename T >
struct polymorphic_type< ::std::weak_ptr< T > >  : polymorphic_type< T >{};

template < typename T >
struct polymorphic_type< ::std::unique_ptr< T > >  : polymorphic_type< T >{};


template < typename T >
struct wire_type< ::std::shared_ptr<T> > : wire_type< typename polymorphic_type< T >::type > {};
template < typename T >
struct wire_type< ::std::weak_ptr<T> > : wire_type< typename polymorphic_type< T >::type > {};
template < typename T >
struct wire_type< ::std::unique_ptr<T> > : wire_type< typename polymorphic_type< T >::type > {};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_DETAIL_WIRE_TRAITS_HPP_ */
