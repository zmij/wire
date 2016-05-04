/*
 * array_io.hpp
 *
 *  Created on: May 4, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_ARRAY_IO_HPP_
#define WIRE_ENCODING_DETAIL_ARRAY_IO_HPP_

#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/wire_traits.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>
#include <algorithm>

namespace wire {
namespace encoding {
namespace detail {

template < typename T, size_t N >
struct array_writer_impl {
    using array_type            = ::std::array<T, N>;
    using in_type               = typename arg_type_helper<array_type>::in_type;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, in_type v)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;

        for (auto const& e : v) {
            write(o, e);
        }
    }
};

template < typename T, size_t N >
struct array_reader_impl {
    using array_type            = ::std::array<T, N>;
    using out_type              = typename arg_type_helper<array_type>::out_type;

    enum {
        size = N
    };

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;

        array_type tmp;
        for (auto& e : tmp) {
            read(begin, end, e);
        }
        ::std::swap(tmp, v);
    }
};

template < typename T >
struct array_writer;
template < typename T >
struct array_reader;

template < typename T, size_t N >
struct array_writer< ::std::array< T, N > > : array_writer_impl<T, N> {};
template < typename T, size_t N >
struct array_reader< ::std::array< T, N > > : array_reader_impl<T, N> {};

}  /* namespace detail */
}  /* namespace encoding */
}  /* namespace wire */



#endif /* WIRE_ENCODING_DETAIL_ARRAY_IO_HPP_ */
