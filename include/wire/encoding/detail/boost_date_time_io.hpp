/*
 * boost_date_time.hpp
 *
 *  Created on: May 12, 2017
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_BOOST_DATE_TIME_IO_HPP_
#define WIRE_ENCODING_DETAIL_BOOST_DATE_TIME_IO_HPP_

#include <boost/date_time/posix_time/posix_time_types.hpp>
//#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>
#include <wire/encoding/detail/struct_io.hpp>
#include <wire/errors/exceptions.hpp>

namespace wire {
namespace encoding {
namespace detail {

template <>
struct struct_writer< ::boost::posix_time::time_duration > {
    using value_type    = ::boost::posix_time::time_duration;
    using in_type       = arg_type_helper< value_type >::in_type;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, in_type v)
    {
        encoding::write(o, v.total_microseconds());
    }
};

template <>
struct struct_reader< ::boost::posix_time::time_duration > {
    using value_type    = ::boost::posix_time::time_duration;
    using out_type      = typename arg_type_helper< value_type >::out_type;

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using microseconds = ::boost::posix_time::microseconds;
        ::std::int64_t ms;
        encoding::read(begin, end, ms);
        v = microseconds{ms};
    }
};

/**
 * Marshaling/unmarshaling for ::boost::posix_time::ptime
 */
template <>
struct struct_writer< ::boost::posix_time::ptime > {
    using value_type    = ::boost::posix_time::ptime;
    using in_type       = arg_type_helper< value_type >::in_type;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, in_type v)
    {
        using microseconds = ::boost::posix_time::microseconds;
        static value_type const epoch{ ::boost::gregorian::date{ 1970, 1, 1 } };
        encoding::write(o, v - epoch);
    }

};

template <>
struct struct_reader< ::boost::posix_time::ptime > {
    using value_type    = ::boost::posix_time::ptime;
    using out_type      = arg_type_helper< value_type >::out_type;

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, out_type v)
    {
        using microseconds = ::boost::posix_time::microseconds;
        static value_type const epoch{ ::boost::gregorian::date{ 1970, 1, 1 } };
        ::std::int64_t ms;
        encoding::read(begin, end, ms);
        v = epoch + microseconds{ms};
    }
};

} /* namespace detail */
} /* namespace encoding */
} /* namespace wire */

#endif /* WIRE_ENCODING_DETAIL_BOOST_DATE_TIME_IO_HPP_ */
