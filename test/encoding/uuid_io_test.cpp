/*
 * uuid_io_test.cpp
 *
 *  Created on: 27 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <wire/encoding/detail/uuid_io.hpp>
#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace encoding {
namespace test {

using buffer_type       = std::vector<uint8_t>;
using input_iterator    = buffer_type::const_iterator;
using output_iterator   = std::back_insert_iterator<buffer_type>;

TEST(UUID, WireIO)
{
    buffer_type buffer;
    boost::uuids::random_generator gen;
    boost::uuids::uuid u = gen();
    EXPECT_NO_THROW(encoding::write(std::back_inserter(buffer), u));
    std::cerr << "UUID buffer size " << buffer.size() << "\n";
    boost::uuids::uuid u1 = boost::uuids::nil_uuid();
    EXPECT_NE(u, u1);
    input_iterator b = buffer.begin();
    input_iterator e = buffer.end();
    EXPECT_NO_THROW(encoding::read(b, e, u1));
    EXPECT_EQ(u, u1);
}

} // namespace test
}  // namespace encoding
}  // namespace wire
