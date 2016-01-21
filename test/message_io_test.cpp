/*
 * message_io_test.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/message.hpp>

namespace wire {
namespace encoding {
namespace test {

TEST(Message, IOTest)
{
	typedef std::vector<uint8_t>						buffer_type;
	typedef buffer_type::const_iterator					input_iterator;
	typedef std::back_insert_iterator<buffer_type>		output_iterator;

	buffer_type buffer;
	message m;
	write(std::back_inserter(buffer), m);
}

}  // namespace test
}  // namespace encoding
}  // namespace wire
