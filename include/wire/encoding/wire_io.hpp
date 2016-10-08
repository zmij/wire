/*
 * wire_io.hpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_WIRE_IO_HPP_
#define WIRE_ENCODING_WIRE_IO_HPP_

#include <wire/encoding/detail/wire_io_detail.hpp>
#include <pushkin/meta/index_tuple.hpp>
#include <tuple>

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

namespace detail {

template < typename OutputIterator, typename T >
void
write_impl(OutputIterator o, T const& value)
{
    write(o, value);
}

template < typename OutputIterator, typename T, typename ... Y >
void
write_impl(OutputIterator o, T const& arg, Y const& ... args)
{
    write(o, arg);
    write_impl(o, args ...);
}

template < typename InputIterator, typename T >
void
read_impl(InputIterator& begin, InputIterator end, T& value)
{
    read(begin, end, value);
}

template < typename InputIterator, typename T, typename ... Y >
void
read_impl(InputIterator& begin, InputIterator end, T& arg, Y& ... args)
{
    read(begin, end, arg);
    read_impl(begin, end, args ...);
}

template < typename InputIterator, size_t ... Indexes, typename ... T >
void
read_impl(InputIterator& begin, InputIterator end,
        ::psst::meta::indexes_tuple< Indexes ... >, std::tuple< T ... >& args)
{
    read_impl(begin, end, std::get<Indexes>(args) ... );
}

}  // namespace detail

template < typename OutputIterator, typename ... T >
void
write(OutputIterator o, T const& ... args)
{
    detail::write_impl(o, args...);
}

template < typename OutputIterator >
void
write(OutputIterator o)
{
    // Effectively do nothing
}

template < typename InputIterator, typename ... T >
void
read(InputIterator& begin, InputIterator end, T& ... args)
{
    detail::read_impl(begin, end, args ...);
}

template < typename InputIterator, typename ... T >
void
read(InputIterator& begin, InputIterator end, std::tuple< T ... >& tuple)
{
    typedef typename ::psst::meta::index_builder< sizeof ... (T) >::type index_type;
    detail::read_impl(begin, end, index_type(), tuple);
}

}  // namespace encoding
}  // namespace wire



#endif /* WIRE_ENCODING_WIRE_IO_HPP_ */
