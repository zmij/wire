/*
 * string_io.hpp
 *
 *  Created on: Jan 21, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_STRING_IO_HPP_
#define WIRE_ENCODING_DETAIL_STRING_IO_HPP_

#include <string>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/varint_io.hpp>

namespace wire {
namespace encoding {
namespace detail {

struct string_writer {
	typedef arg_type_helper< std::string >::in_type	in_type;
	typedef varint_writer< std::size_t, false >		size_writer;

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		typedef octet_output_iterator_concept< OutputIterator >	output_iterator_check;
		typedef typename output_iterator_check::value_type		value_type;

		size_writer::output(o, v.size());
		std::copy(v.begin(), v.end(), o);
	}
};

struct string_reader {
	typedef arg_type_helper< std::string >::base_type	base_type;
	typedef arg_type_helper< std::string >::out_type	out_type;
	typedef varint_reader< std::size_t, false >			size_reader;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		typedef octet_input_iterator_concept< InputIterator >	input_iterator_check;

		std::size_t str_size;
		size_reader::input(begin, end, str_size);
		base_type val;
		val.resize(str_size);
		if (!copy_max(begin, end, val.begin(), str_size)) {
			throw errors::unmarshal_error("Failed to read string contents");
		}
		std::swap(v, val);
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire


#endif /* WIRE_ENCODING_DETAIL_STRING_IO_HPP_ */
