/*
 * variant_io_test.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/detail/variant_io.hpp>
#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace encoding {
namespace test {

TEST(Variant, IO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef boost::variant<int, std::string, float> variant_type;

	buffer_type buffer;

	variant_type iv(10);
	variant_type sv("Tha string");
	variant_type fv(3.14f);

	write(std::back_inserter(buffer), iv);
	std::cerr << "Buffer size " << buffer.size() << "\n";
	write(std::back_inserter(buffer), sv);
	std::cerr << "Buffer size " << buffer.size() << "\n";
	write(std::back_inserter(buffer), fv);
	std::cerr << "Buffer size " << buffer.size() << "\n";

	input_iterator b = buffer.begin();
	input_iterator e = buffer.end();

	variant_type v1;
	read(b, e, v1);
	EXPECT_EQ(iv, v1);
	std::cerr << "Variant value read: " << v1 << "\n";
	read(b, e, v1);
	std::cerr << "Variant value read: " << v1 << "\n";
	EXPECT_EQ(sv, v1);
	read(b, e, v1);
	std::cerr << "Variant value read: " << v1 << "\n";
	EXPECT_EQ(fv, v1);
}



}  // namespace test
}  // namespace encoding
}  // namespace wire

