/*
 * buffers_test.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/buffers.hpp>
#include <wire/encoding/detail/fixed_size_io.hpp>

#include <vector>
#include <type_traits>

namespace wire {
namespace encoding {
namespace test {

template< typename T >
class buffer_io_test : public ::testing::TestWithParam< T > {
protected:
	typedef std::vector<uint8_t>						buffer_type;
	typedef buffer_type::const_iterator					input_iterator;
	typedef std::back_insert_iterator<buffer_type>		output_iterator;

	typedef detail::writer< T >	writer_type;
	typedef detail::reader< T >	reader_type;

	buffer_type buffer;
};

#define BUFFER_IO_TEST(the_type, generator) \
typedef buffer_io_test< the_type > the_type##_io_test; \
TEST_P(the_type##_io_test, IOTest) \
{ \
	ParamType v = GetParam(); \
	ParamType e; \
\
	writer_type::output(std::back_inserter(buffer), v); \
	std::cerr << "Value " << v << " Buffer size " << buffer.size() << "\n"; \
	auto begin = buffer.begin(); \
	reader_type::input(begin, buffer.end(), e); \
	EXPECT_EQ(v, e); \
	EXPECT_EQ(begin, buffer.end()); \
} \
INSTANTIATE_TEST_CASE_P(BufferIO, the_type##_io_test, generator)

//@{
/** @name Varint io */
BUFFER_IO_TEST(uint16_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
		std::numeric_limits<uint16_t>::max())
);

BUFFER_IO_TEST(int16_t,
	::testing::Values(-1, -5, -1800, 0, 1, 2, 16, 100, 4096,
		std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min())
);

BUFFER_IO_TEST(uint32_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
		std::numeric_limits<uint32_t>::max()));

BUFFER_IO_TEST(int32_t,
	::testing::Values(0, 1, 2, -1, -5, 16, 100, -1800, 4096,
		std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()));

BUFFER_IO_TEST(uint64_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
			std::numeric_limits<uint32_t>::max(),
			std::numeric_limits<uint64_t>::max()));

BUFFER_IO_TEST(int64_t,
	::testing::Values(0, 1, 2, -1, -5, 16, 100, -1800, 4096,
		std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(),
		std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min()));

typedef detail::wire_types wire_types_enum;
BUFFER_IO_TEST(wire_types_enum,
	::testing::Values(
		detail::SCALAR_VARINT, detail::SCALAR_FIXED)
);

enum class test_enumeration {
	val0, val2 = 2, val3
};
std::ostream&
operator << (std::ostream& os, test_enumeration val)
{
	std::ostream::sentry s(os);
	if (s) {
		os << static_cast<std::underlying_type<test_enumeration>::type>(val);
	}
	return os;
}

BUFFER_IO_TEST(test_enumeration,
	::testing::Values(
		test_enumeration::val0, test_enumeration::val2, test_enumeration::val3)
);

//@}

//@{
/** @name Fixed size values */
BUFFER_IO_TEST(bool, ::testing::Values(true, false));
BUFFER_IO_TEST(char, ::testing::Values(0, '0', 'a', 'B', '=', '*'));
typedef unsigned char uchar;
BUFFER_IO_TEST(uchar, ::testing::Values(0, '0', 'a', 'B', '=', '*'));
BUFFER_IO_TEST(uint8_t, ::testing::Values(0, '0', 'a', 'B', '=', '*'));
BUFFER_IO_TEST(int8_t, ::testing::Values(0, '0', 'a', 'B', '=', '*'));

BUFFER_IO_TEST(float, ::testing::Values(0, 1, -3.14,
		std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
		std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::lowest()));

BUFFER_IO_TEST(double, ::testing::Values(0, 1, -3.14,
		std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
		std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::lowest(),
		std::numeric_limits<double>::min(), std::numeric_limits<double>::max(),
		std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::lowest()));

typedef long double long_double;
BUFFER_IO_TEST(long_double, ::testing::Values(0, 1, -3.14,
		std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
		std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::lowest(),
		std::numeric_limits<double>::min(), std::numeric_limits<double>::max(),
		std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::lowest(),
		std::numeric_limits<long double>::min(), std::numeric_limits<long double>::max(),
		std::numeric_limits<long double>::epsilon(), std::numeric_limits<long double>::lowest()));

BUFFER_IO_TEST(uint32_fixed_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
		std::numeric_limits<uint32_t>::max()));
BUFFER_IO_TEST(int32_fixed_t,
		::testing::Values(0, 1, 2, -1, -5, 16, 100, -1800, 4096,
			std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()));

BUFFER_IO_TEST(uint64_fixed_t,
	::testing::Values(0, 1, 2, 4, 8, 16, 100, 1024, 4096,
			std::numeric_limits<uint32_t>::max(),
			std::numeric_limits<uint64_t>::max()));

BUFFER_IO_TEST(int64_fixed_t,
	::testing::Values(0, 1, 2, -1, -5, 16, 100, -1800, 4096,
		std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(),
		std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min()));
//@}

//@{
/** @name string io */
typedef std::string std_string;
BUFFER_IO_TEST(std_string,
	::testing::Values(
			"", " ", "abcdABCD", "+-!@#%^&", "абвгАБВГ",
			"こんにちはテスト",  "メッセージ",  "안녕하세요 테스트", "你好测试",
			"Waſſerschloſʒ", "Wasserschloß"
	)
);
//@}

}  // namespace test
}  // namespace encoding
}  // namespace wire
