/*
 * identity_io_test.cpp
 *
 *  Created on: 27 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/core/identity.hpp>

namespace wire {
namespace core {
namespace test {

using buffer_type       = ::std::vector<uint8_t>;
using input_iterator    = buffer_type::const_iterator;
using output_iterator   = ::std::back_insert_iterator<buffer_type>;

TEST(IO, Identity)
{
    buffer_type buffer;
    identity id0{ identity::random("test") };
    EXPECT_NO_THROW(encoding::write(::std::back_inserter(buffer), id0));
    ::std::cerr << "Identity buffer size " << buffer.size() << "\n";
    identity id1;
    EXPECT_NE(id0, id1);
    input_iterator b = buffer.begin();
    input_iterator e = buffer.end();
    EXPECT_NO_THROW(encoding::read(b, e, id1));
    EXPECT_EQ(id0, id1);
}

} // namespace test
}  // namespace core
}  // namespace wire
