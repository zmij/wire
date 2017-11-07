/*
 * boost_date_time_io_test.cpp
 *
 *  Created on: May 12, 2017
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/detail/boost_date_time_io.hpp>

#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace encoding {
namespace test {

using buffer_type       = std::vector<uint8_t>;
using input_iterator    = buffer_type::const_iterator;
using output_iterator   = std::back_insert_iterator<buffer_type>;

TEST(DateTime, BoostIO)
{
    namespace posix_time = ::boost::posix_time;
    using clock         = posix_time::microsec_clock;
    using time_point    = posix_time::ptime;
    using duration      = posix_time::time_duration;
    using microseconds  = posix_time::microseconds;

    auto p1 = clock::universal_time();
    auto p2 = p1 - posix_time::hours{24};
    {
        time_point p_out;
        buffer_type buffer;
        EXPECT_NO_THROW(encoding::write(::std::back_inserter(buffer), p1));
        ::std::cerr << "Boost ptime buffer size " << buffer.size() << "\n";

        EXPECT_NE(p1, p_out);
        input_iterator b = buffer.begin();
        input_iterator e = buffer.end();
        EXPECT_NO_THROW(encoding::read(b, e, p_out));
        EXPECT_GT(microseconds(1), p1 - p_out);
    }
    {
        auto diff = p1 - p2;
        duration d_out;
        buffer_type buffer;
        EXPECT_NO_THROW(encoding::write(::std::back_inserter(buffer), diff));
        ::std::cerr << "Boost duration buffer size " << buffer.size() << "\n";
        EXPECT_NE(diff, d_out);
        input_iterator b = buffer.begin();
        input_iterator e = buffer.end();
        EXPECT_NO_THROW(encoding::read(b, e, d_out));
        EXPECT_EQ(diff, d_out);
    }
}

} /* namespace test */
} /* namespace encoding */
} /* namespace wire */

