/*
 * struct_io.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_STRUCT_IO_HPP_
#define WIRE_ENCODING_DETAIL_STRUCT_IO_HPP_

#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/wire_traits.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
struct struct_writer {
	typedef typename arg_type_helper<T>::in_type	in_type;

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator >	output_iterator_check;
		wire_write(o, v);
	}
};

template < typename T >
struct struct_reader {
	typedef typename arg_type_helper<T>::out_type	out_type;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;
		wire_read(begin, end, v);
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_DETAIL_STRUCT_IO_HPP_ */
