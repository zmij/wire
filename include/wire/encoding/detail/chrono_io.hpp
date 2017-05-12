/*
 * chrono_io.hpp
 *
 *  Created on: May 11, 2017
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_CHRONO_IO_HPP_
#define WIRE_ENCODING_DETAIL_CHRONO_IO_HPP_

#include <chrono>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>
#include <wire/encoding/detail/struct_io.hpp>
#include <wire/errors/exceptions.hpp>

namespace wire {
namespace encoding {
namespace detail {

/**
 * Serialization for ::std::chrono::duration
 */
template < typename Rep, typename Period >
struct struct_writer< ::std::chrono::duration<Rep, Period> > {
    using value_type    = ::std::chrono::duration<Rep, Period>;
    using in_type       = typename arg_type_helper< value_type >::in_type;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, in_type v)
    {
        using microseconds = ::std::chrono::microseconds;
        microseconds ms = ::std::chrono::duration_cast< microseconds >(v);
        encoding::write(o, ms.count());
    }
};

template < typename Rep, typename Period >
struct struct_reader< ::std::chrono::duration<Rep, Period> > {
    using value_type    = ::std::chrono::duration<Rep, Period>;
    using out_type      = typename arg_type_helper< value_type >::out_type;

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using microseconds = ::std::chrono::microseconds;
        microseconds::rep rep;
        encoding::read(begin, end, rep);
        v = ::std::chrono::duration_cast< value_type >(microseconds{rep});
    }
};

/**
 * Marshaling/unmarshaling for ::std::chrono time point
 * Resolution is downscaled to microseconds for interoperability with
 * ::boost::date_time where nanosecond resolution is not always available.
 */
template <>
struct struct_writer< ::std::chrono::system_clock::time_point > {
    using value_type    = ::std::chrono::system_clock::time_point;
    using in_type       = arg_type_helper< value_type >::in_type;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, in_type v)
    {
        using duration = ::std::chrono::microseconds;
        duration ms = ::std::chrono::duration_cast< duration >( v.time_since_epoch() );
        encoding::write(o, ms);
    }
};

template <>
struct struct_reader< ::std::chrono::system_clock::time_point > {
    using value_type    = ::std::chrono::system_clock::time_point;
    using out_type      = arg_type_helper< value_type >::out_type;

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using duration = ::std::chrono::microseconds;
        static value_type const epoch;

        duration ms;
        encoding::read(begin, end, ms);
        v = epoch + ms;
    }
};

} /* namespace detail */
} /* namespace encoding */
} /* namespace wire */


#endif /* WIRE_ENCODING_DETAIL_CHRONO_IO_HPP_ */
