/*
 * uuid_io.hpp
 *
 *  Created on: 26 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_ENCODING_DETAIL_UUID_IO_HPP_
#define WIRE_ENCODING_DETAIL_UUID_IO_HPP_

#include <boost/uuid/uuid.hpp>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>
#include <wire/encoding/detail/struct_io.hpp>
#include <wire/errors/exceptions.hpp>

namespace wire {
namespace encoding {
namespace detail {

//template <>
//struct wire_type< boost::uuids::uuid > : std::integral_constant< wire_types, SCALAR_FIXED > {};

template <>
struct struct_writer< boost::uuids::uuid > {
	typedef arg_type_helper< boost::uuids::uuid >::in_type	in_type;

	template < typename OutputIterator >
	static void
	output(OutputIterator o, in_type v)
	{
		std::copy(v.begin(), v.end(), o);
	}
};

template <>
struct struct_reader< boost::uuids::uuid > {
	typedef arg_type_helper< boost::uuids::uuid >::out_type	out_type;

	template < typename InputIterator >
	static void
	input(InputIterator& begin, InputIterator end, out_type v)
	{
		if (!copy_max(begin, end, v.begin(), boost::uuids::uuid::static_size()))
			throw errors::unmarshal_error("Failed to read uuid from input stream");
	}
};

}  // namespace detail
}  // namespace encoding
}  // namespace wire

#endif /* WIRE_ENCODING_DETAIL_UUID_IO_HPP_ */
