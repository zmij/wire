/*
 * optional_io_test.cpp
 *
 *  Created on: 29 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/encoding/detail/optional_io.hpp>
#include <wire/encoding/wire_io.hpp>

#include <boost/optional/optional_io.hpp>

namespace wire {
namespace encoding {
namespace test {

TEST(Optional, IO)
{
    using buffer_type       = std::vector<uint8_t>;
    using input_iterator    =  buffer_type::const_iterator;
    using output_iterator   = std::back_insert_iterator<buffer_type>;
    using optional_string   = ::boost::optional< ::std::string >;

    buffer_type buffer;

    optional_string empty;
    optional_string lipsum{"Lorem ipsum dolor sit amet"};

    write(::std::back_inserter(buffer), empty);
    std::cerr << "Buffer size " << buffer.size() << "\n";
    write(::std::back_inserter(buffer), lipsum);
    std::cerr << "Buffer size " << buffer.size() << "\n";

    input_iterator b = buffer.begin();
    input_iterator e = buffer.end();

    optional_string s1;
    read(b, e, s1);
    EXPECT_EQ(empty, s1);
    read(b, e, s1);
    EXPECT_EQ(lipsum, s1);
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */
