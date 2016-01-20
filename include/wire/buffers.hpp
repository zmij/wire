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

#include <boost/endian/arithmetic.hpp>

#include <bitset>
#include <iostream>

namespace wire {

namespace detail {

template < typename T, size_t byte >
struct varint_mask_impl {
	typedef T type;
	static constexpr type value =
			(static_cast< type >(0xff) << (byte * 8)) |
			varint_mask_impl< T, byte - 1 >::value;
};

template < typename T >
struct varint_mask_impl< T, 0 > {
	typedef T type;
	static constexpr type value = static_cast<type>(1 << 7);
};

template < typename T >
struct varint_mask : varint_mask_impl< T, sizeof(T) - 1 > {};

template < typename T, bool is_signed >
struct varint_writer;

template < typename T >
struct varint_writer < T, true > {
	typedef typename arg_type_helper<T>::type		type;
	typedef typename std::make_unsigned<type>::type	unsigned_type;
	typedef varint_writer< unsigned_type, false >	unsigned_writer;

	enum {
		bit_count = sizeof(type) * 8 - 1
	};


	template < typename OutputIterator >
	static void
	write( OutputIterator o, type v)
	{
		unsigned_writer::write(o,
			static_cast< unsigned_type >( (v << 1) ^ (v >> bit_count) ));
	}
};

template < typename OutputIterator >
struct output_iterator_traits : std::iterator_traits< OutputIterator > {};

template < typename Container >
struct output_iterator_traits< std::back_insert_iterator< Container > >
	: std::iterator_traits< typename Container::iterator > {};

template < typename T >
struct varint_writer < T, false > {
	typedef typename arg_type_helper<T>::type type;
	typedef varint_mask< T >	mask_type;

	static constexpr type seven_bits = static_cast<type>(0x7f); // 0b01111111
	static constexpr type eighth_bit = static_cast<type>(1 << 7); // 0b10000000

	template < typename OutputIterator >
	static void
	write( OutputIterator o, type v)
	{
		typedef OutputIterator							iterator_type;
		typedef output_iterator_traits< iterator_type >	iterator_traits;
		typedef typename iterator_traits::value_type	value_type;

		static_assert(sizeof(value_type) == 1,
			"Output iterator should be octet-based");

		v = boost::endian::native_to_little(v);
		value_type current = v & seven_bits;
		while (v & mask_type::value) {
			current |= eighth_bit;
			*o++ = current;
			v = v >> 7;
			current = v & seven_bits;
		}
		*o++ = current;
	}
};

template < typename T, bool is_signed >
struct varint_reader;

template < typename T >
struct zig_zag_traits {
	typedef typename std::decay<T>::type			type;
	typedef typename std::make_unsigned<type>::type	unsigned_type;
	enum {
		bit_count = sizeof(type) * 8 - 1
	};
};

template <>
struct zig_zag_traits<int16_t> {
	typedef int16_t									type;
	typedef typename std::make_unsigned<type>::type	unsigned_type;
	enum {
		bit_count = 31
	};
};

template < typename T >
struct varint_reader< T, true > {
	typedef typename std::decay<T>::type			type;
	typedef typename std::make_unsigned<type>::type	unsigned_type;
	typedef varint_reader< unsigned_type, false >	unsigned_reader;
	typedef zig_zag_traits<type>					traits;

	enum {
		bit_count = traits::bit_count
	};

	template < typename InputIterator >
	static bool
	read(InputIterator begin, InputIterator end, type& v)
	{
		unsigned_type tmp;
		if (unsigned_reader::read(begin, end, tmp)) {
			v = static_cast<type>(tmp >> 1) ^
					(static_cast<type>(tmp) << bit_count >> bit_count);
			return true;
		}
		return false;
	}
};

template < typename T >
struct varint_reader< T, false > {
	typedef typename std::decay<T>::type			type;
	typedef varint_mask< type >	mask_type;

	static constexpr type seven_bits = static_cast<type>(0x7f); // 0b01111111
	static constexpr type eighth_bit = static_cast<type>(1 << 7); // 0b10000000

	template < typename InputIterator >
	static bool
	read(InputIterator begin, InputIterator end, type& v)
	{
		typedef InputIterator							iterator_type;
		typedef std::iterator_traits< iterator_type >	iterator_traits;
		typedef typename iterator_traits::value_type	value_type;

		static_assert(sizeof(value_type) == 1,
			"Input iterator should be octet-based");

		type tmp = 0;
		bool more = true;
		for (uint32_t n = 0; more && begin != end; ++n) {
			type curr_byte = *begin++;
			tmp |= (curr_byte & seven_bits) << (7 * n);
			more = curr_byte & eighth_bit;
		}
		if (more) {
			return false;
		}
		v = boost::endian::little_to_native(tmp);
		return true;
	}
};

template < typename T, wire_types >
struct writer {
	typedef typename arg_type_helper<T>::type type;

	template< typename OutputIterator >
	static void
	write( OutputIterator, type );
};

template < typename T >
struct writer< T, SCALAR_VARINT > :
	varint_writer< T, std::is_signed< T >::value > {};

template < typename T, wire_types >
struct reader {
	typedef typename std::decay<T>::type type;

	template < typename InputIterator >
	static void
	read(InputIterator begin, InputIterator end, type& v);
};

template < typename T >
struct reader < T, SCALAR_VARINT > :
	varint_reader< T, std::is_signed< T >::value > {};

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
		typedef detail::wire_type< type > wire_type;
		typedef detail::writer< T, wire_type::value > writer_type;

		writer_type::write(current_, v);
	}
	template < typename T >
	void
	write(T&& v)
	{
		typedef typename std::decay<T>::type type;
		typedef detail::wire_type< type > wire_type;
		typedef detail::writer< T, wire_type::value > writer_type;

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
