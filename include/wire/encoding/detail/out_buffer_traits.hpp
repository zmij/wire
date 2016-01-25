/*
 * out_buffer_traits.hpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_OUT_BUFFER_TRAITS_HPP_
#define WIRE_ENCODING_DETAIL_OUT_BUFFER_TRAITS_HPP_

#include <vector>

namespace wire {
namespace encoding {
class outgoing;

namespace detail {

template < typename T >
struct out_buffer_traits;

template <>
struct out_buffer_traits< uint8_t* > {
	typedef outgoing const*					container_pointer;
	typedef std::vector<uint8_t>			buffer_type;
	typedef std::vector<buffer_type>		buffers_sequence_type;

	typedef buffer_type::pointer			pointer;
	typedef buffer_type::reference			reference;

	typedef buffers_sequence_type::iterator	buffer_iterator_type;
	typedef buffer_type::iterator			value_iterator_type;
};

template <>
struct out_buffer_traits< uint8_t const* > {
	typedef outgoing const*					container_pointer;
	typedef std::vector<uint8_t>			buffer_type;
	typedef std::vector<buffer_type>		buffers_sequence_type;

	typedef buffer_type::const_pointer		pointer;
	typedef buffer_type::const_reference	reference;
	typedef buffers_sequence_type::const_iterator
											buffer_iterator_type;
	typedef buffer_type::const_iterator		value_iterator_type;
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_DETAIL_OUT_BUFFER_TRAITS_HPP_ */
