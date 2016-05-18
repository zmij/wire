/*
 * arrays_io_test.cpp
 *
 *  Created on: Feb 4, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/wire_io.hpp>
#include <iostream>

namespace wire {
namespace encoding {
namespace test {

TEST(IO, Vector)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::vector< ::std::string >;
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

TEST(IO, CharVector)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::vector< char >;
    {
        buffer_type buffer;
        std::string str{"string one"};
        array_type in_value{ str.begin(), str.end() };
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

TEST(IO, List)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::list< ::std::string >;
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


TEST(IO, Deque)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::deque< ::std::string >;
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

TEST(IO, Set)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::set< ::std::string >;
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

TEST(IO, Multiset)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::multiset< ::std::string >;
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

TEST(IO, UnorderedSet)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::unordered_set< ::std::string >;
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

TEST(IO, UnorderedMultiset)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::unordered_multiset< ::std::string >;
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

TEST(IO, Map)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::map< int, ::std::string >;
    {
        buffer_type buffer;
        array_type in_value{
            { 1, "string one"},
            { 2, "string two"},
            { 3, "string three"},
            { 4, "string four"},
            { 5, "string five"},
            { 6, "string six"},
            { 7, "string seven"},
            { 8, "string eight"},
            { 9, "string nine"},
            { 10, "string ten"},
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

TEST(IO, UnorderedMap)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::unordered_map< int, ::std::string >;
    {
        buffer_type buffer;
        array_type in_value{
            { 1, "string one"},
            { 2, "string two"},
            { 3, "string three"},
            { 4, "string four"},
            { 5, "string five"},
            { 6, "string six"},
            { 7, "string seven"},
            { 8, "string eight"},
            { 9, "string nine"},
            { 10, "string ten"},
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

TEST(IO, Multimap)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::multimap< int, ::std::string >;
    {
        buffer_type buffer;
        array_type in_value{
            { 1, "string one"},
            { 2, "string two"},
            { 3, "string three"},
            { 4, "string four"},
            { 5, "string five"},
            { 6, "string six"},
            { 7, "string seven"},
            { 8, "string eight"},
            { 9, "string nine"},
            { 10, "string ten"},
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

TEST(IO, UnorderedMultimap)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::unordered_multimap< int, ::std::string >;
    {
        buffer_type buffer;
        array_type in_value{
            { 1, "string one"},
            { 2, "string two"},
            { 3, "string three"},
            { 4, "string four"},
            { 5, "string five"},
            { 6, "string six"},
            { 7, "string seven"},
            { 8, "string eight"},
            { 9, "string nine"},
            { 10, "string ten"},
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

TEST(IO, Array)
{
    using buffer_type = std::vector<uint8_t>;
    using input_iterator = buffer_type::const_iterator;
    using output_iterator = std::back_insert_iterator<buffer_type>;
    using array_type = ::std::array< double, 4 >;
    {
        buffer_type buffer;
        array_type in_value{{ 1, 2, 3, 4}};
        EXPECT_NO_THROW(write(std::back_inserter(buffer), in_value));
        ::std::cerr << "Buffer size " << buffer.size() << "\n";
        array_type out_value{{ 0, 0, 0, 0}};
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
