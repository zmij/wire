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
	message m1{ message::request, 123 };
	message m2;
	EXPECT_NO_THROW(write(std::back_inserter(buffer), m1));
	EXPECT_NE(0, buffer.size());
	std::cerr << "Message buffer size " << buffer.size() << "\n";
	auto begin = buffer.begin();

	EXPECT_NO_THROW(read(begin, buffer.end(), m2));
	EXPECT_EQ(m1, m2);
	EXPECT_EQ(begin, buffer.end());

	begin = buffer.begin();
	EXPECT_THROW(read(begin, begin + buffer.size() - 2, m2), errors::unmarshal_error);
	begin = buffer.begin() + 1;
	EXPECT_THROW(read(begin, buffer.end(), m2), errors::invalid_magic_number);
}

}  // namespace test
}  // namespace encoding
}  // namespace wire
