/*
 * helpers.hpp
 *
 *  Created on: 20 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_DETAIL_HELPERS_HPP_
#define WIRE_DETAIL_HELPERS_HPP_

namespace wire {
namespace detail {

template < typename T, bool is_fundamental >
struct arg_type_helper_impl;

template < typename T >
struct arg_type_helper_impl< T, true > {
	typedef T type;
};

template < typename T >
struct arg_type_helper_impl< T, false > {
	typedef T const& type;
};

template < typename T >
struct arg_type_helper :
	arg_type_helper_impl< typename std::decay< T >::type,
			std::is_fundamental< typename std::decay< T >::type >::value > {};

template < typename OutputIterator >
struct output_iterator_traits : std::iterator_traits< OutputIterator > {};

template < typename Container >
struct output_iterator_traits< std::back_insert_iterator< Container > >
	: std::iterator_traits< typename Container::iterator > {};

}  // namespace detail
}  // namespace wire



#endif /* WIRE_DETAIL_HELPERS_HPP_ */
