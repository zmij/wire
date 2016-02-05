/*
 * vector_io.hpp
 *
 *  Created on: Feb 4, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_CONTAINERS_IO_HPP_
#define WIRE_ENCODING_DETAIL_CONTAINERS_IO_HPP_

#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/wire_traits.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
struct add_via_insert : ::std::false_type {};

template < typename Elem, typename ... Rest >
struct add_via_insert< ::std::set<Elem, Rest ...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct add_via_insert< ::std::multiset<Elem, Rest ...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct add_via_insert< ::std::unordered_set<Elem, Rest ...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct add_via_insert< ::std::unordered_multiset<Elem, Rest ...> > : ::std::true_type {};
template < typename Key, typename Value, typename ... Rest >
struct add_via_insert< ::std::map<Key, Value, Rest ...> > : ::std::true_type {};
template < typename Key, typename Value, typename ... Rest >
struct add_via_insert< ::std::multimap<Key, Value, Rest ...> > : ::std::true_type {};
template < typename Key, typename Value, typename ... Rest >
struct add_via_insert< ::std::unordered_map<Key, Value, Rest ...> > : ::std::true_type {};
template < typename Key, typename Value, typename ... Rest >
struct add_via_insert< ::std::unordered_multimap<Key, Value, Rest ...> > : ::std::true_type {};

template < typename T >
struct can_reserve : ::std::false_type {};

template < typename Elem, typename ... Rest >
struct can_reserve< ::std::vector<Elem, Rest...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct can_reserve< ::std::unordered_set<Elem, Rest...> > : ::std::true_type {};
template < typename Elem, typename ... Rest >
struct can_reserve< ::std::unordered_multiset<Elem, Rest...> > : ::std::true_type {};
template < typename Key, typename Value, typename ... Rest >
struct can_reserve< ::std::unordered_map< Key, Value, Rest ...> > : ::std::true_type {};
template < typename Key, typename Value, typename ... Rest >
struct can_reserve< ::std::unordered_multimap< Key, Value, Rest ...> > : ::std::true_type {};

template < typename T >
struct is_dictionary : ::std::false_type {};

template < typename K, typename V, typename ... Rest >
struct is_dictionary< ::std::map<K, V, Rest ...> > : ::std::true_type {};
template < typename K, typename V, typename ... Rest >
struct is_dictionary< ::std::multimap<K, V, Rest ...> > : ::std::true_type {};
template < typename K, typename V, typename ... Rest >
struct is_dictionary< ::std::unordered_map<K, V, Rest ...> > : ::std::true_type {};
template < typename K, typename V, typename ... Rest >
struct is_dictionary< ::std::unordered_multimap<K, V, Rest ...> > : ::std::true_type {};

template < typename T, bool >
struct reserve_traits_impl;

template < typename T >
struct reserve_traits_impl< T, true > {
	typedef T									container_type;
	typedef typename container_type::size_type	size_type;

	static void
	reserve(container_type& c, size_type sz)
	{
		c.reserve(sz);
	}
};

template < typename T >
struct reserve_traits_impl< T, false > {
	typedef T									container_type;
	typedef typename container_type::size_type	size_type;

	static void
	reserve(container_type& c, size_type sz) {}
};

template < typename T >
struct reserve_traits : reserve_traits_impl< T, can_reserve<T>::value > {};

template < typename T, bool >
struct add_traits_impl;

template < typename T >
struct add_traits_impl< T, true > {
	typedef T									container_type;
	typedef typename container_type::value_type	element_type;

	static void
	add(container_type& c, element_type&& e)
	{
		c.insert(std::move(e));
	}
};

template < typename T >
struct add_traits_impl< T, false > {
	typedef T									container_type;
	typedef typename container_type::value_type	element_type;

	static void
	add(container_type& c, element_type&& e)
	{
		c.push_back(std::move(e));
	}
};

template < typename T >
struct add_traits : add_traits_impl< T, add_via_insert< T >::value > {};

template < typename T >
struct container_traits {
	typedef T									container_type;
	typedef typename container_type::size_type	size_type;
	typedef typename container_type::value_type	element_type;
	typedef reserve_traits< T >					reserve_type;
	typedef add_traits< T >						add_type;

	static void
	reserve(container_type& c, size_type sz)
	{
		reserve_type::reserve(c, sz);
	}
	static void
	add(container_type& c, element_type&& e)
	{
		add_type::add(c, ::std::move(e));
	}
};

template < typename T, bool is_byte >
struct container_writer_impl;

template < typename T >
struct container_writer_impl< T, false > {
	typedef T													container_type;
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
struct container_writer_impl< T, true > {
	typedef T													container_type;
	typedef typename container_type::size_type					size_type;
	typedef typename arg_type_helper<container_type>::in_type	in_type;

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator >	output_iterator_check;
		size_type sz = v.size();
		write(o, sz);
		std::copy(v.begin(), v.end(), o);
	}
};

template < typename T >
struct container_writer;

template < template<typename, typename ...> class Container,
	typename Element, typename ... Rest >
struct container_writer< Container<Element, Rest ...> >
	: container_writer_impl< Container<Element, Rest ... >, sizeof(Element) == 1 > {};

template < typename T, bool is_byte >
struct container_reader_impl;

template < typename T >
struct container_reader_impl< T, false > {
	typedef T													container_type;
	typedef typename container_type::value_type					element_type;
	typedef typename container_type::size_type					size_type;
	typedef typename arg_type_helper<container_type>::out_type	out_type;
	typedef container_traits<container_type>					traits;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;
		size_type sz;
		read(begin, end, sz);
		if (sz > 0) {
			container_type tmp;
			traits::reserve(tmp, sz);
			for (size_type i = 0; i < sz; ++i) {
				element_type e;
				read(begin, end, e);
				traits::add(tmp, std::move(e));
			}
			std::swap(v, tmp);
		}
	}
};

template < typename T >
struct container_reader_impl< T, true > {
	typedef T													container_type;
	typedef typename container_type::value_type					element_type;
	typedef typename container_type::size_type					size_type;
	typedef typename arg_type_helper<container_type>::out_type	out_type;
	typedef container_traits<container_type>					traits;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;
		size_type sz;
		read(begin, end, sz);
		if (sz > 0) {
			container_type tmp;
			copy_max(begin, end, std::back_inserter(tmp), sz);
			std::swap(v, tmp);
		}
	}
};

template < typename T >
struct container_reader;

template < template<typename, typename ...> class Container,
	typename Element, typename ... Rest >
struct container_reader< Container<Element, Rest ...> > :
	container_reader_impl< Container<Element, Rest ...>, sizeof(Element) == 1 > {};

template < typename T >
struct dictionary_writer;

template < template<typename, typename, typename ...> class Dictionary,
	typename Key, typename Value, typename ... Rest >
struct dictionary_writer< Dictionary<Key, Value, Rest ...> > {
	typedef Dictionary<Key, Value, Rest ...>						dictionary_type;
	typedef typename dictionary_type::size_type						size_type;
	typedef typename arg_type_helper< dictionary_type >::in_type	in_type;

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
struct dictionary_reader;

template < template<typename, typename, typename ...> class Dictionary,
	typename Key, typename Value, typename ... Rest >
struct dictionary_reader< Dictionary<Key, Value, Rest ...> > {
	typedef Dictionary<Key, Value, Rest ...>						dictionary_type;
	typedef Key														key_type;
	typedef Value													value_type;
	typedef std::pair< key_type, value_type >						element_type;
	typedef typename dictionary_type::size_type						size_type;
	typedef typename arg_type_helper< dictionary_type >::out_type	out_type;
	typedef container_traits<dictionary_type>						traits;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;
		size_type sz;
		read(begin, end, sz);
		if (sz > 0) {
			dictionary_type tmp;
			traits::reserve(tmp, sz);
			for (size_type i = 0; i < sz; ++i) {
				element_type e;
				read(begin, end, e);
				traits::add(tmp, std::move(e));
			}
			std::swap(v, tmp);
		}
	}
};


}  // namespace detail
}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_DETAIL_CONTAINERS_IO_HPP_ */
