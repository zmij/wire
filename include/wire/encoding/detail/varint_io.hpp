/*
 * varint.hpp
 *
 *  Created on: 20 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_DETAIL_VARINT_IO_HPP_
#define WIRE_DETAIL_VARINT_IO_HPP_

#include <wire/encoding/detail/helpers.hpp>
#include <boost/endian/arithmetic.hpp>

namespace wire {
namespace encoding {
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

/**
 * Implementation of a varint writer for signed integer types,
 * using zig-zag encoding.
 */
template < typename T >
struct varint_writer < T, true > {
	typedef typename arg_type_helper<T>::in_type			in_type;
	typedef typename arg_type_helper<T>::base_type			base_type;
	typedef typename std::make_unsigned<base_type>::type	unsigned_type;
	typedef varint_writer< unsigned_type, false >			unsigned_writer;

	enum {
		shift_bits = sizeof(in_type) * 8 - 1
	};


	template < typename OutputIterator >
	static void
	write( OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator > output_iterator_check;
		typedef typename output_iterator_check::value_type		value_type;

		unsigned_writer::write(o,
			static_cast< unsigned_type >( (v << 1) ^ (v >> shift_bits) ));
	}
};

/**
 * Implementation of varint writer for unsigned types
 */
template < typename T >
struct varint_writer < T, false > {
	typedef typename arg_type_helper<T>::base_type	base_type;
	typedef typename arg_type_helper<T>::in_type	in_type;
	typedef varint_mask< T >	mask_type;

	static constexpr base_type seven_bits = static_cast<base_type>(0x7f); // 0b01111111
	static constexpr base_type eighth_bit = static_cast<base_type>(1 << 7); // 0b10000000

	template < typename OutputIterator >
	static void
	write( OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator >	output_iterator_check;
		typedef typename output_iterator_check::value_type		value_type;

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
		shift_bits = sizeof(type) * 8 - 1
	};
};

template <>
struct zig_zag_traits<int16_t> {
	typedef int16_t									type;
	typedef typename std::make_unsigned<type>::type	unsigned_type;
	enum {
		shift_bits = 31
	};
};

template < typename T >
struct varint_reader< T, true > {
	typedef typename std::decay<T>::type			type;
	typedef typename std::make_unsigned<type>::type	unsigned_type;
	typedef varint_reader< unsigned_type, false >	unsigned_reader;
	typedef zig_zag_traits<type>					traits;

	enum {
		shift_bits = traits::shift_bits
	};

	template < typename InputIterator >
	static std::pair< InputIterator, bool >
	read(InputIterator begin, InputIterator end, type& v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;
		typedef typename input_iterator_check::result_type		result_type;

		unsigned_type tmp;
		result_type res = unsigned_reader::read(begin, end, tmp);
		if (res.second) {
			v = static_cast<type>(tmp >> 1) ^
					(static_cast<type>(tmp) << shift_bits >> shift_bits);
			return res;
		}
		return std::make_pair(begin, false);
	}
};

template < typename T >
struct varint_reader< T, false > {
	typedef typename arg_type_helper<T>::base_type			base_type;
	typedef typename arg_type_helper<T>::out_type			out_type;
	typedef varint_mask< base_type >	mask_type;

	static constexpr base_type seven_bits = static_cast<base_type>(0x7f); // 0b01111111
	static constexpr base_type eighth_bit = static_cast<base_type>(1 << 7); // 0b10000000

	template < typename InputIterator >
	static std::pair< InputIterator, bool >
	read(InputIterator begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;
		typedef typename input_iterator_check::result_type		result_type;

		auto start = begin;
		base_type tmp = 0;
		bool more = true;
		for (uint32_t n = 0; more && begin != end; ++n) {
			base_type curr_byte = *begin++;
			tmp |= (curr_byte & seven_bits) << (7 * n);
			more = curr_byte & eighth_bit;
		}
		if (more) {
			return std::make_pair(start, false);
		}
		v = boost::endian::little_to_native(tmp);
		return std::make_pair(begin, true);
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_DETAIL_VARINT_IO_HPP_ */
