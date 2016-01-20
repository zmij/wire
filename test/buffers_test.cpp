/*
 * buffers_test.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/buffers.hpp>
#include <vector>

namespace wire {
namespace test {

template< typename T >
class varint_io_test : public ::testing::TestWithParam< T > {
protected:
	typedef std::vector<uint8_t>						buffer_type;
	typedef buffer_type::const_iterator					input_iterator;
	typedef std::back_insert_iterator<buffer_type>		output_iterator;

	typedef detail::writer< T, detail::SCALAR_VARINT >	writer_type;
	typedef detail::reader< T, detail::SCALAR_VARINT >	reader_type;

	buffer_type buffer;
};

#define VARINT_TEST(int_type, generator) \
typedef varint_io_test< int_type > var##int_type##_test; \
TEST_P(var##int_type##_test, IOTest) \
{ \
	ParamType v = GetParam(); \
	ParamType e = 0; \
\
	writer_type::write(std::back_inserter(buffer), v); \
	std::cerr << "Value " << v << " Buffer size " << buffer.size() << "\n"; \
	std::cerr << "v\t" << std::bitset<32>(v) << "\n";\
	reader_type::read(buffer.begin(), buffer.end(), e); \
	EXPECT_EQ(v, e); \
} \
INSTANTIATE_TEST_CASE_P(VarintIO, var##int_type##_test, generator)

VARINT_TEST(uint16_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
		std::numeric_limits<uint16_t>::max())
);

VARINT_TEST(int16_t,
	::testing::Values(-1, -5, -1800, 0, 1, 2, 16, 100, 4096,
		std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min())
);

VARINT_TEST(uint32_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
		std::numeric_limits<uint32_t>::max()));

VARINT_TEST(int32_t,
	::testing::Values(0, 1, 2, -1, -5, 16, 100, -1800, 4096,
		std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()));

VARINT_TEST(uint64_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
			std::numeric_limits<uint32_t>::max(),
			std::numeric_limits<uint64_t>::max()));

VARINT_TEST(int64_t,
	::testing::Values(0, 1, 2, -1, -5, 16, 100, -1800, 4096,
		std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(),
		std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min()));

}  // namespace test
}  // namespace wire
