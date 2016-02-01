/*
 * out_buffer_traits.hpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_BUFFER_TRAITS_HPP_
#define WIRE_ENCODING_DETAIL_BUFFER_TRAITS_HPP_

#include <vector>

namespace wire {
namespace encoding {
class buffer;

namespace detail {

template < typename T >
struct buffer_traits;

template <>
struct buffer_traits< uint8_t* > {
	typedef buffer*						container_pointer;
	typedef std::vector<uint8_t>			buffer_type;
	typedef std::vector<buffer_type>		buffers_sequence_type;

	typedef buffer_type::pointer			pointer;
	typedef buffer_type::reference			reference;

	typedef buffers_sequence_type::iterator	buffer_iterator_type;
	typedef buffer_type::iterator			value_iterator_type;
};

template <>
struct buffer_traits< uint8_t const* > {
	typedef buffer const*					container_pointer;
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


#endif /* WIRE_ENCODING_DETAIL_BUFFER_TRAITS_HPP_ */
