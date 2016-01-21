/*
 * helpers.hpp
 *
 *  Created on: 20 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_DETAIL_HELPERS_HPP_
#define WIRE_DETAIL_HELPERS_HPP_

#include <wire/encoding/types.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T, bool is_fundamental >
struct arg_type_helper_impl;

template < typename T >
struct arg_type_helper_impl< T, true > {
	typedef typename std::decay< T >::type	base_type;
	typedef base_type						in_type;
	typedef base_type&						out_type;
};

template < typename T, bool is_enum >
struct arg_type_helper_enum;

template < typename T >
struct arg_type_helper_enum< T, true > {
	typedef typename std::decay< T >::type	base_type;
	typedef base_type						in_type;
	typedef base_type&						out_type;
};

template < typename T >
struct arg_type_helper_enum< T, false > {
	typedef typename std::decay< T >::type	base_type;
	typedef base_type const&				in_type;
	typedef base_type&						out_type;
};

template < typename T >
struct arg_type_helper_impl< T, false >
	: arg_type_helper_enum< T, std::is_enum<T>::value > {};

template < typename T >
struct arg_type_helper :
	arg_type_helper_impl< typename std::decay< T >::type,
			std::is_fundamental< typename std::decay< T >::type >::value > {};

template < typename T >
struct arg_type_helper< fixed_size< T > > {
	typedef fixed_size< T >		base_type;
	typedef base_type			in_type;
	typedef base_type&			out_type;
};

template < typename OutputIterator >
struct output_iterator_traits : std::iterator_traits< OutputIterator > {};

template < typename Container >
struct output_iterator_traits< std::back_insert_iterator< Container > >
	: std::iterator_traits< typename Container::iterator > {};

template < typename OutputIterator >
struct octet_output_iterator_concept {
	typedef OutputIterator							iterator_type;
	typedef output_iterator_traits< iterator_type >	iterator_traits;
	typedef typename iterator_traits::value_type	value_type;

	static_assert(sizeof(value_type) == 1,
		"Output iterator should be octet-based");
};

template < typename InputIterator >
struct octet_input_iterator_concept {
	typedef InputIterator							iterator_type;
	typedef std::iterator_traits< iterator_type >	iterator_traits;
	typedef typename iterator_traits::value_type	value_type;
	static_assert(sizeof(value_type) == 1,
			"Input iterator should be octet-based");
};

template < typename InputIterator, typename OutputIterator >
bool
copy_max(InputIterator& p, InputIterator e, OutputIterator o, std::size_t max)
{
	std::size_t copied = 0;
	for (; p != e && copied < max; ++copied) {
		*o++ = *p++;
	}
	return copied == max;
}

}  // namespace detail
}  // namespace encoding
}  // namespace wire



#endif /* WIRE_DETAIL_HELPERS_HPP_ */
