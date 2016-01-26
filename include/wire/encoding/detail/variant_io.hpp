/*
 * variant_io.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_VARIANT_IO_HPP_
#define WIRE_ENCODING_DETAIL_VARIANT_IO_HPP_

#include <boost/variant.hpp>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/varint_io.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>
#include <wire/util/meta_helpers.hpp>

#include <functional>
#include <array>

namespace wire {
namespace encoding {
namespace detail {

template < typename ... T >
struct wire_type< boost::variant< T ... > > : std::integral_constant< wire_types, VARIANT > {};

template < typename OutputIterator, typename VariantType, size_t num, typename T >
struct write_nth_type {
	typedef OutputIterator										iterator_type;
	typedef T													value_type;
	typedef writer_impl< value_type,
			wire_type<value_type>::value >						writer_type;
	typedef VariantType											variant_type;
	typedef typename arg_type_helper< variant_type >::in_type	in_type;

	static void
	output(iterator_type out, in_type v)
	{
		value_type const& val = boost::get<value_type>(v);
		writer_type::output(out, val);
	}
};

template < typename OutputIterator, typename IndexesTuple, typename ... T >
struct variant_write_unwrapper_base;

template < typename OutputIterator, std::size_t ... Indexes, typename ... T >
struct variant_write_unwrapper_base< OutputIterator, util::indexes_tuple< Indexes ... >, T ... > {
	typedef boost::variant< T ... >								variant_type;
	typedef typename arg_type_helper< variant_type >::in_type	in_type;
	typedef OutputIterator										iterator_type;
	typedef std::function< void( iterator_type, in_type ) >		write_func_type;
	enum {
		size = sizeof ... (T)
	};
	typedef std::array< write_func_type, size >					func_table_type;

	static func_table_type&
	functions()
	{
		static func_table_type table_{
			&write_nth_type< iterator_type, variant_type, Indexes, T >::output ...
		};
		return table_;
	}

	static void
	output_nth(iterator_type out, in_type v, std::size_t type_idx)
	{
		assert(type_idx < size && "Variant type index is in bounds of type list");
		functions()[type_idx](out, v);
	}
};

template < typename OutputIterator, typename ... T >
struct variant_write_unwrapper :
	variant_write_unwrapper_base< OutputIterator, typename util::index_builder< sizeof ... (T) >::type, T ... > {};


template < typename ... T >
struct writer< boost::variant< T ... > > {
	typedef boost::variant< T ... >								variant_type;
	typedef typename arg_type_helper< variant_type >::in_type	in_type;
	typedef varint_writer< std::size_t, false >					type_writer;

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator >	output_iterator_check;
		typedef variant_write_unwrapper< OutputIterator, T ... > write_unwrapper;

		type_writer::output(o, v.which());
		write_unwrapper::output_nth(o, v, v.which());
	}
};

template < typename InputIterator, typename VariantType, size_t num, typename T >
struct read_nth_type {
	typedef InputIterator										iterator_type;
	typedef T													value_type;
	typedef reader_impl< value_type,
			wire_type<value_type>::value >						reader_type;
	typedef VariantType											variant_type;
	typedef typename arg_type_helper< variant_type >::out_type	out_type;

	static void
	input(iterator_type& begin, iterator_type end, out_type v)
	{
		value_type val;
		reader_type::input(begin, end, val);
		v = val;
	}
};

template < typename InputIterator, typename IndexesTuple, typename ... T >
struct variant_read_unwrapper_base;

template < typename InputIterator, std::size_t ... Indexes, typename ... T >
struct variant_read_unwrapper_base< InputIterator, util::indexes_tuple< Indexes ... >, T ... > {
	typedef boost::variant< T ... >								variant_type;
	typedef typename arg_type_helper< variant_type >::out_type	out_type;
	typedef InputIterator										iterator_type;
	typedef std::function<
		void( iterator_type&, iterator_type, out_type ) >		read_func_type;
	enum {
		size = sizeof ... (T)
	};
	typedef std::array< read_func_type, size >					func_table_type;

	static func_table_type&
	functions()
	{
		static func_table_type table_{
			&read_nth_type< iterator_type, variant_type, Indexes, T >::input ...
		};
		return table_;
	}

	static void
	input_nth(iterator_type& begin, iterator_type end, out_type v, std::size_t type_idx)
	{
		assert(type_idx < size && "Variant type index is in bounds of type list");
		functions()[type_idx](begin, end, v);
	}
};

template < typename InputIterator, typename ... T >
struct variant_read_unwrapper :
	variant_read_unwrapper_base< InputIterator, typename util::index_builder< sizeof ... (T) >::type, T ... > {};

template < typename ... T >
struct reader< boost::variant< T ... > > {
	typedef boost::variant< T ... >								variant_type;
	typedef typename arg_type_helper< variant_type >::out_type	out_type;
	typedef varint_reader< std::size_t, false >					type_reader;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;
		typedef variant_read_unwrapper< InputIterator, T ... >	read_unwrapper;

		std::size_t type_idx(0);
		type_reader::input(begin, end, type_idx);
		read_unwrapper::input_nth(begin, end, v, type_idx);
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_DETAIL_VARIANT_IO_HPP_ */

