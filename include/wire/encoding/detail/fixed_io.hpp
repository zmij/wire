/*
 * fixed_io.hpp
 *
 *  Created on: 20 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_DETAIL_FIXED_IO_HPP_
#define WIRE_DETAIL_FIXED_IO_HPP_

#include <wire/encoding/types.hpp>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/errors/exceptions.hpp>

#include <boost/endian/arithmetic.hpp>

#include <algorithm>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
struct fixed_size_writer {
	typedef typename arg_type_helper< T >::in_type		in_type;
	enum {
		byte_count = sizeof(T)
	};

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator > output_iterator_check;

		char* p = reinterpret_cast<char*>(&v);
		char* e = p + byte_count;
		std::copy(p, e, o);
	}
};

template < typename T >
struct fixed_size_writer<fixed_size< T >> {
	typedef typename arg_type_helper<fixed_size< T >>::in_type in_type;
	enum {
		byte_count = in_type::size
	};

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator > output_iterator_check;

		v.value = boost::endian::native_to_little(v.value);
		char const* p = reinterpret_cast<char const*>(&v.value);
		char const* e = p + byte_count;
		std::copy(p, e, o);
	}
};

template < typename T >
struct fixed_size_reader {
	typedef typename arg_type_helper< T >::base_type	base_type;
	typedef typename arg_type_helper< T >::out_type		out_type;
	enum {
		byte_count = sizeof(base_type)
	};

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;

		auto start = begin;
		base_type tmp;
		char* p = reinterpret_cast<char*>(&tmp);
		if (copy_max(begin, end, p, byte_count)) {
			std::swap(v, tmp);
			return;
		}

		throw errors::unmarshal_error("Failed to unmarshal fixed type");
	}
};

template < typename T >
struct fixed_size_reader<fixed_size< T >> {
	typedef typename arg_type_helper<fixed_size< T >>::base_type	base_type;
	typedef typename arg_type_helper<fixed_size< T >>::out_type		out_type;
	typedef typename base_type::type								fundamental_type;
	enum {
		byte_count = base_type::size
	};

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;

		auto start = begin;
		fundamental_type tmp;
		char* p = reinterpret_cast<char*>(&tmp);
		if (copy_max(begin, end, p, byte_count)) {
			v.value = boost::endian::little_to_native(tmp);
			return;
		}

		throw errors::unmarshal_error("Failed to unmarshal fixed type");
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire



#endif /* WIRE_DETAIL_FIXED_IO_HPP_ */
