/*
 * fixed_io.hpp
 *
 *  Created on: 20 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_DETAIL_FIXED_IO_HPP_
#define WIRE_DETAIL_FIXED_IO_HPP_

#include <wire/types.hpp>
#include <wire/detail/helpers.hpp>
#include <boost/endian/arithmetic.hpp>
#include <algorithm>

namespace wire {
namespace detail {

template < typename T >
struct fixed_size_writer {
	typedef typename arg_type_helper< T >::type				type;
	enum {
		byte_count = sizeof(T)
	};

	template < typename OutputIterator >
	static void
	write(OutputIterator o, type v)
	{
		typedef OutputIterator							iterator_type;
		typedef output_iterator_traits< iterator_type >	iterator_traits;
		typedef typename iterator_traits::value_type	value_type;

		static_assert(sizeof(value_type) == 1,
			"Output iterator should be octet-based");

		char* p = reinterpret_cast<char*>(&v);
		char* e = p + byte_count;
		std::copy(p, e, o);
	}
};

template < typename T >
struct fixed_size_writer<fixed_size< T >> {
	typedef typename arg_type_helper<fixed_size< T >>::type type;
	enum {
		byte_count = type::size
	};

	template < typename OutputIterator >
	static void
	write(OutputIterator o, type v)
	{
		typedef OutputIterator							iterator_type;
		typedef output_iterator_traits< iterator_type >	iterator_traits;
		typedef typename iterator_traits::value_type	value_type;

		static_assert(sizeof(value_type) == 1,
			"Output iterator should be octet-based");

		v.value = boost::endian::native_to_little(v.value);
		char* p = reinterpret_cast<char*>(&v.value);
		char* e = p + byte_count;
		std::copy(p, e, o);
	}
};

template < typename T >
struct fixed_size_reader {
	typedef typename arg_type_helper< T >::type type;
	enum {
		byte_count = sizeof(type)
	};

	template < typename InputIterator >
	static std::pair< InputIterator, bool >
	read(InputIterator begin, InputIterator end, type& v)
	{
		typedef InputIterator							iterator_type;
		typedef std::iterator_traits< iterator_type >	iterator_traits;
		typedef typename iterator_traits::value_type	value_type;

		static_assert(sizeof(value_type) == 1,
			"Input iterator should be octet-based");

		auto start = begin;
		type tmp;
		char* p = reinterpret_cast<char*>(&tmp);
		if (copy_max(begin, end, p, byte_count)) {
			v = tmp;
			return std::make_pair(begin, true);
		}

		return std::make_pair(start, false);
	}
};

template < typename T >
struct fixed_size_reader<fixed_size< T >> {
	typedef typename arg_type_helper<fixed_size< T >>::type type;
	typedef typename type::type								base_type;
	enum {
		byte_count = type::size
	};

	template < typename InputIterator >
	static std::pair< InputIterator, bool >
	read(InputIterator begin, InputIterator end, type& v)
	{
		typedef InputIterator							iterator_type;
		typedef std::iterator_traits< iterator_type >	iterator_traits;
		typedef typename iterator_traits::value_type	value_type;

		static_assert(sizeof(value_type) == 1,
			"Input iterator should be octet-based");

		auto start = begin;
		base_type tmp;
		char* p = reinterpret_cast<char*>(&tmp);
		if (copy_max(begin, end, p, byte_count)) {
			v.value = boost::endian::little_to_native(tmp);
			return std::make_pair(begin, true);
		}

		return std::make_pair(start, false);
	}
};

}  // namespace detail
}  // namespace wire



#endif /* WIRE_DETAIL_FIXED_IO_HPP_ */
