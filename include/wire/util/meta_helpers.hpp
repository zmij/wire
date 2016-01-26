/*
 * meta_helpers.hpp
 *
 *  Created on: Jul 21, 2015
 *      Author: zmij
 */

#ifndef WIRE_UTIL_META_HELPERS_HPP_
#define WIRE_UTIL_META_HELPERS_HPP_

namespace wire {
namespace util {

/**
 * Metafunction for calculating Nth type in variadic template parameters
 */
template < size_t num, typename ... T >
struct nth_type;

template < size_t num, typename T, typename ... Y >
struct nth_type< num, T, Y ... > : nth_type< num - 1, Y ...> {
};

template < typename T, typename ... Y >
struct nth_type < 0, T, Y ... > {
	typedef T type;
};

template < size_t ... Indexes >
struct indexes_tuple {
	enum {
		size = sizeof ... (Indexes)
	};
};

template < size_t num, typename tp = indexes_tuple <> >
struct index_builder;

template < size_t num, size_t ... Indexes >
struct index_builder< num, indexes_tuple< Indexes ... > >
	: index_builder< num - 1, indexes_tuple< Indexes ..., sizeof ... (Indexes) > > {
};

template <size_t ... Indexes >
struct index_builder< 0, indexes_tuple< Indexes ... > > {
	typedef indexes_tuple < Indexes ... > type;
	enum {
		size = sizeof ... (Indexes)
	};
};

}  // namespace util
}  // namespace wire

#endif /* WIRE_UTIL_META_HELPERS_HPP_ */
