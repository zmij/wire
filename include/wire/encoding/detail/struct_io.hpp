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
		write(o, v);
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_DETAIL_STRUCT_IO_HPP_ */
