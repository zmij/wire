/*
 * vector_io.hpp
 *
 *  Created on: Feb 4, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_ARRAYS_IO_HPP_
#define WIRE_ENCODING_DETAIL_ARRAYS_IO_HPP_

#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/wire_traits.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
struct is_set_container : ::std::false_type {};

template < typename Elem, typename ... Rest >
struct is_set_container< ::std::set<Elem, Rest ...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct is_set_container< ::std::multiset<Elem, Rest ...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct is_set_container< ::std::unordered_set<Elem, Rest ...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct is_set_container< ::std::unordered_multiset<Elem, Rest ...> > : ::std::true_type {};

template < typename T, bool >
struct container_traits_impl;

template < typename T >
struct container_traits_impl< T, false > {
	typedef T									container_type;
	typedef typename container_type::size_type	size_type;
	typedef typename container_type::value_type	element_type;

	static void
	reserve(container_type& c, size_type sz) {}

	static void
	add(container_type& c, element_type&& e)
	{
		c.push_back(std::move(e));
	}
};

template < typename T >
struct container_traits_impl< T, true > {
	typedef T									container_type;
	typedef typename container_type::size_type	size_type;
	typedef typename container_type::value_type	element_type;

	static void
	reserve(container_type& c, size_type sz) {}

	static void
	add(container_type& c, element_type&& e)
	{
		c.insert(std::move(e));
	}
};

template < typename T >
struct container_traits : container_traits_impl< T, is_set_container< T >::value > {};

template < typename Element, typename ... Rest >
struct container_traits< ::std::vector<Element, Rest ...> > {
	typedef ::std::vector< Element, Rest ... >	container_type;
	typedef typename container_type::size_type	size_type;
	typedef typename container_type::value_type	element_type;

	static void
	reserve(container_type& c, size_type sz)
	{
		c.reserve(sz);
	}

	static void
	add(container_type& c, element_type&& e)
	{
		c.push_back(std::move(e));
	}
};

template < typename Element, typename ... Rest >
struct container_traits< ::std::queue<Element, Rest ...> > {
	typedef ::std::queue< Element, Rest ... >	container_type;
	typedef typename container_type::size_type	size_type;
	typedef typename container_type::value_type	element_type;

	static void
	reserve(container_type& c, size_type sz) {}

	static void
	add(container_type& c, element_type&& e)
	{
		c.push(std::move(e));
	}
};

template < typename T >
struct container_writer;

// TODO Specializations for char and unsigned char
template < template<typename, typename ...> class Container,
	typename Element, typename ... Rest >
struct container_writer< Container<Element, Rest ...> > {
	typedef Container<Element, Rest ...>						container_type;
	typedef typename container_type::size_type					size_type;
	typedef typename arg_type_helper<container_type>::in_type	in_type;

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator >	output_iterator_check;
		size_type sz = v.size();
		write(o, sz);
		for (auto const& e : v) {
			write(o, e);
		}
	}
};

template < typename T >
struct container_reader;

// TODO Specializations for char and unsigned char
template < template<typename, typename ...> class Container,
	typename Element, typename ... Rest >
struct container_reader< Container<Element, Rest ...> > {
	typedef Container<Element, Rest ...>						container_type;
	typedef Element												element_type;
	typedef typename container_type::size_type					size_type;
	typedef typename arg_type_helper<container_type>::out_type	out_type;
	typedef container_traits<container_type>					traits;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		container_type tmp;
		size_type sz;
		read(begin, end, sz);
		if (sz > 0)
			traits::reserve(tmp, sz);
		for (size_type i = 0; i < sz; ++i) {
			element_type e;
			read(begin, end, e);
			traits::add(tmp, std::move(e));
		}
		std::swap(v, tmp);
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_DETAIL_ARRAYS_IO_HPP_ */
