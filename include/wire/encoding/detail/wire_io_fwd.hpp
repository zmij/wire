/*
 * wire_io_fwd.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_WIRE_IO_FWD_HPP_
#define WIRE_ENCODING_DETAIL_WIRE_IO_FWD_HPP_

#include <wire/encoding/detail/wire_traits.hpp>
#include <wire/encoding/detail/helpers.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T, wire_types >
struct writer_impl;
template < typename T, wire_types >
struct reader_impl;

template < typename T >
struct writer : writer_impl< T, wire_type<T>::value> {};

template < typename T >
struct reader : reader_impl< T, wire_type<T>::value> {};

}  // namespace detail

template < typename OutputIterator, typename T >
void
write(OutputIterator o, T v, typename std::enable_if< std::is_fundamental<T>::value, T >::type* = nullptr);
template < typename OutputIterator, typename T >
void
write(OutputIterator o, T const& v, typename std::enable_if< !std::is_fundamental<T>::value, T >::type* = nullptr);
template < typename OutputIterator, typename ... T >
void
write(OutputIterator o, T const& ... args);

template < typename InputIterator, typename T >
void
read(InputIterator& begin, InputIterator end, T& value);
template < typename InputIterator, typename ... T >
void
read(InputIterator& begin, InputIterator end, T& ... args);

}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_DETAIL_WIRE_IO_FWD_HPP_ */
