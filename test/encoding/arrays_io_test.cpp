/*
 * arrays_io_test.cpp
 *
 *  Created on: Feb 4, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace encoding {
namespace test {

TEST(Containers, VectorIO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef ::std::vector< ::std::string > array_type;
	{
		buffer_type buffer;
		array_type in_value{
			"string one",
			"string two",
			"string three",
			"string four",
			"string five",
			"string six",
			"string seven",
			"string eight",
			"string nine",
			"string ten",
		};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
	{
		buffer_type buffer;
		array_type in_value{};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
}

TEST(Containers, ListIO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef ::std::list< ::std::string > array_type;
	{
		buffer_type buffer;
		array_type in_value{
			"string one",
			"string two",
			"string three",
			"string four",
			"string five",
			"string six",
			"string seven",
			"string eight",
			"string nine",
			"string ten",
		};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
	{
		buffer_type buffer;
		array_type in_value{};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
}


TEST(Containers, DequeIO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef ::std::deque< ::std::string > array_type;
	{
		buffer_type buffer;
		array_type in_value{
			"string one",
			"string two",
			"string three",
			"string four",
			"string five",
			"string six",
			"string seven",
			"string eight",
			"string nine",
			"string ten",
		};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
	{
		buffer_type buffer;
		array_type in_value{};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
}

TEST(Containers, SetIO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef ::std::set< ::std::string > array_type;
	{
		buffer_type buffer;
		array_type in_value{
			"string one",
			"string two",
			"string three",
			"string four",
			"string five",
			"string six",
			"string seven",
			"string eight",
			"string nine",
			"string ten",
		};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
	{
		buffer_type buffer;
		array_type in_value{};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
}

TEST(Containers, MultisetIO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef ::std::multiset< ::std::string > array_type;
	{
		buffer_type buffer;
		array_type in_value{
			"string one",
			"string two",
			"string three",
			"string four",
			"string five",
			"string six",
			"string seven",
			"string eight",
			"string nine",
			"string ten",
		};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
	{
		buffer_type buffer;
		array_type in_value{};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
}

TEST(Containers, UnorderedSetIO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef ::std::unordered_set< ::std::string > array_type;
	{
		buffer_type buffer;
		array_type in_value{
			"string one",
			"string two",
			"string three",
			"string four",
			"string five",
			"string six",
			"string seven",
			"string eight",
			"string nine",
			"string ten",
		};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
	{
		buffer_type buffer;
		array_type in_value{};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
}

TEST(Containers, UnorderedMultisetIO)
{
	typedef std::vector<uint8_t> 					buffer_type;
	typedef buffer_type::const_iterator				input_iterator;
	typedef std::back_insert_iterator<buffer_type>	output_iterator;
	typedef ::std::unordered_multiset< ::std::string > array_type;
	{
		buffer_type buffer;
		array_type in_value{
			"string one",
			"string two",
			"string three",
			"string four",
			"string five",
			"string six",
			"string seven",
			"string eight",
			"string nine",
			"string ten",
		};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
	{
		buffer_type buffer;
		array_type in_value{};
		EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
		array_type out_value;
		input_iterator b = buffer.begin();
		input_iterator e = buffer.end();
		EXPECT_NO_THROW(read(b, e, out_value));
		EXPECT_EQ(in_value.size(), out_value.size());
		EXPECT_EQ(in_value, out_value);
	}
}

}  // namespace test
}  // namespace encoding
}  // namespace wire
