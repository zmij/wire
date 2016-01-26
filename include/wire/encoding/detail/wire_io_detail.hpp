/*
 * wire_io_detail.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_WIRE_IO_DETAIL_HPP_
#define WIRE_ENCODING_DETAIL_WIRE_IO_DETAIL_HPP_

#include <wire/encoding/detail/wire_io_fwd.hpp>
#include <wire/encoding/detail/fixed_io.hpp>
#include <wire/encoding/detail/varint_io.hpp>
#include <wire/encoding/detail/string_io.hpp>
#include <wire/encoding/detail/struct_io.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
struct writer_impl< T, SCALAR_FIXED >
	: fixed_size_writer< T > {};
template < typename T >
struct reader_impl< T, SCALAR_FIXED >
	: fixed_size_reader< T > {};

template < typename T >
struct writer_impl<T, SCALAR_VARINT >
	: varint_writer< T, std::is_signed< typename std::decay<T>::type >::value > {};
template < typename T >
struct reader_impl< T, SCALAR_VARINT >
	: varint_reader< T, std::is_signed< typename std::decay<T>::type >::value > {};

template <>
struct writer_impl< std::string, SCALAR_WITH_SIZE >
	: string_writer {};
template <>
struct reader_impl< std::string, SCALAR_WITH_SIZE >
	: string_reader {};

template < typename T >
struct writer_impl < T, STRUCT >
	: struct_writer< T > {};

template < typename T >
struct writer : writer_impl< T, wire_type<T>::value> {};

template < typename T >
struct reader : reader_impl< T, wire_type<T>::value> {};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_DETAIL_WIRE_IO_DETAIL_HPP_ */
