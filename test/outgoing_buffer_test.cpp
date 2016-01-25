/*
 * ougoing_buffer_test.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/buffers.hpp>

namespace wire {
namespace encoding {
namespace test {

TEST(OutgoingBuffer, Construction)
{
	outgoing out;
	EXPECT_TRUE(out.empty());
	EXPECT_EQ(0, out.size());
}

TEST(OutgoingBuffer, Iterators)
{

}

}  // namespace test
}  // namespace encoding
}  // namespace wire

