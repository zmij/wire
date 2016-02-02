/*
 * buffers.inl
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_BUFFERS_INL_
#define WIRE_ENCODING_BUFFERS_INL_

#include <wire/encoding/buffers.hpp>
#include <wire/encoding/detail/helpers.hpp>

namespace wire {
namespace encoding {

template < typename InputIterator >
incoming::incoming(message const& m, InputIterator& begin, InputIterator end)
{
	create_pimpl(m);
	insert_back(begin, end);
}

template < typename InputIterator >
void
incoming::insert_back(InputIterator& begin, InputIterator end)
{
	size_type sz = size();
	if (header().size > sz) {
		detail::copy_max(begin, end, std::back_inserter(back_buffer()),
				header().size - sz);
		std::copy(begin, end, std::back_inserter(back_buffer()));
	}
}

}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_BUFFERS_INL_ */
