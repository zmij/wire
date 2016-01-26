/*
 * wire_io.hpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_WIRE_IO_HPP_
#define WIRE_ENCODING_DETAIL_WIRE_IO_HPP_

#include <wire/encoding/detail/wire_traits.hpp>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/fixed_io.hpp>
#include <wire/encoding/detail/varint_io.hpp>
#include <wire/encoding/detail/string_io.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T, wire_types >
struct writer_impl;
template < typename T, wire_types >
struct reader_impl;

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
struct writer : writer_impl< T, wire_type<T>::value> {};

template < typename T >
struct reader : reader_impl< T, wire_type<T>::value> {};

}  // namespace detail

template < typename OutputIterator, typename T >
void
write(OutputIterator o, T v)
{
	typedef detail::writer< typename std::decay< T >::type > writer_type;
	writer_type::write(o, v);
}

template < typename InputIterator, typename T >
void
read(InputIterator& begin, InputIterator end, T& value)
{
	typedef detail::reader< typename std::decay< T >::type > reader_type;
	reader_type::read(begin, end, value);
}

}  // namespace encoding
}  // namespace wire



#endif /* WIRE_ENCODING_DETAIL_WIRE_IO_HPP_ */
