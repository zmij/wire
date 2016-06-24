/*
 * optional_io.hpp
 *
 *  Created on: 29 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_ENCODING_DETAIL_OPTIONAL_IO_HPP_
#define WIRE_ENCODING_DETAIL_OPTIONAL_IO_HPP_

#include <boost/optional.hpp>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/fixed_io.hpp>
#include <wire/encoding/detail/wire_io_detail.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
struct writer < ::boost::optional< T > > {
    using optional_type     = ::boost::optional< T >;
    using in_type           = typename arg_type_helper< optional_type >::in_type;
    using flag_writer       = writer< bool >;
    using type_writer       = writer< T >;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, in_type v)
    {
        using output_iterator_check =  octet_output_iterator_concept< OutputIterator >;
        flag_writer::output(o, v.is_initialized());
        if (v.is_initialized()) {
            type_writer::output(o, *v);
        }
    }
};

template < typename T >
struct reader < ::boost::optional< T > > {
    using optional_type     = ::boost::optional< T >;
    using out_type          = typename arg_type_helper< optional_type >::out_type;
    using flag_reader       = reader< bool >;
    using type_reader       = reader< T >;

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;
        bool has_value = false;
        flag_reader::input(begin, end, has_value);
        if (has_value) {
            T val;
            type_reader::input(begin, end, val);
            v = val;
        }
    }
};

}  /* namespace detail */
}  /* namespace encoding */
}  /* namespace wire */


#endif /* WIRE_ENCODING_DETAIL_OPTIONAL_IO_HPP_ */
