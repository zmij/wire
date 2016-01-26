/*
 * wire_io.hpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_WIRE_IO_HPP_
#define WIRE_ENCODING_WIRE_IO_HPP_

#include <wire/encoding/detail/wire_io_detail.hpp>
#include <wire/encoding/detail/variant_io.hpp>

namespace wire {
namespace encoding {

template < typename OutputIterator, typename T >
void
write(OutputIterator o, T v, typename std::enable_if< std::is_fundamental<T>::value, T >::type*)
{
	typedef detail::writer< typename std::decay< T >::type > writer_type;
	writer_type::output(o, v);
}

template < typename OutputIterator, typename T >
void
write(OutputIterator o, T const& v, typename std::enable_if< !std::is_fundamental<T>::value, T >::type*)
{
	typedef detail::writer< typename std::decay< T >::type > writer_type;
	writer_type::output(o, v);
}

template < typename InputIterator, typename T >
void
read(InputIterator& begin, InputIterator end, T& value)
{
	typedef detail::reader< typename std::decay< T >::type > reader_type;
	reader_type::input(begin, end, value);
}

}  // namespace encoding
}  // namespace wire



#endif /* WIRE_ENCODING_WIRE_IO_HPP_ */
