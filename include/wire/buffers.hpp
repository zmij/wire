/*
 * buffers.hpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#ifndef TIP_WIRE_BUFFERS_HPP_
#define TIP_WIRE_BUFFERS_HPP_

#include <type_traits>
#include <iterator>
#include <wire/detail/wire_traits.hpp>
#include <wire/detail/helpers.hpp>
#include <wire/detail/varint.hpp>

#include <bitset>
#include <iostream>

namespace wire {

namespace detail {

template < typename T, wire_types >
struct writer_impl;

template < typename T >
struct writer_impl<T, SCALAR_VARINT >
	: varint_writer< T, std::is_signed< typename std::decay<T>::type >::value > {};

template < typename T, wire_types >
struct reader_impl;

template < typename T >
struct reader_impl< T, SCALAR_VARINT >
	: varint_reader< T, std::is_signed< typename std::decay<T>::type >::value > {};

template < typename T >
struct writer : writer_impl< T, wire_type<T>::value> {};

template < typename T >
struct reader : reader_impl< T, wire_type<T>::value> {};

}  // namespace detail

template < typename OutputIterator >
class outgoing {
public:
	typedef OutputIterator iterator_type;
public:
	outgoing(iterator_type out) : current_(out) {};
public:
	//@{
	/** @name Varint types */
	void
	write(int16_t);
	void
	write(int32_t);
	void
	write(int64_t);
	void
	write(uint16_t);
	void
	write(uint32_t);
	void
	write(uint64_t);
	//@}
	//@{
	void
	write(float);
	void
	write(double);
	//@}

	template < typename T >
	void
	write(T v)
	{
		typedef typename std::decay<T>::type type;
		typedef detail::writer< type > writer_type;

		writer_type::write(current_, v);
	}
	template < typename T >
	void
	write(T&& v)
	{
		typedef typename std::decay<T>::type type;
		typedef detail::writer< type > writer_type;

		writer_type::write(current_, v);
	}
private:
	outgoing(outgoing const&) = delete;
	outgoing&
	operator = (outgoing const&) = delete;
private:
	iterator_type current_;
};

template < typename InputIterator >
class incoming {
public:
	typedef InputIterator iterator_type;
public:
	incoming(iterator_type begin, iterator_type end);
private:
	incoming(incoming const&) = delete;
	incoming&
	operator = (incoming const&) = delete;
private:
	iterator_type current_;
	iterator_type end_;
};

}  // namespace wire


#endif /* TIP_WIRE_BUFFERS_HPP_ */
