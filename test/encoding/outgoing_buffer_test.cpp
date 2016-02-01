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

namespace {

const int INSERT_CHARS = 100;

}  // namespace

TEST(OutgoingBuffer, Construction)
{
	buffer out;
	EXPECT_TRUE(out.empty());
	EXPECT_EQ(0, out.size());

	out.push_back(1);
	EXPECT_EQ(1, out.size());
	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	EXPECT_EQ(INSERT_CHARS + 1, out.size());
	out.pop_back();
	EXPECT_EQ(INSERT_CHARS, out.size());
}

TEST(OutgoingBuffer, ForwardIterators)
{
	buffer out;
	buffer::iterator b = out.begin();
	buffer::iterator e = out.end();
	buffer::const_iterator cb = out.begin();
	buffer::const_iterator ce = out.end();

	EXPECT_EQ(e, b);
	EXPECT_EQ(0, b - e);
	EXPECT_EQ(ce, cb);
	EXPECT_EQ(0, cb - ce);

	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	b = out.begin();
	e = out.end();
	ASSERT_NE(b, e);
	EXPECT_EQ(INSERT_CHARS, e - b);
	EXPECT_EQ(-INSERT_CHARS, b - e);
	{
		int steps = 0;
		for (buffer::iterator p = b; p != e; ++p, ++steps) {}
		EXPECT_EQ(INSERT_CHARS, steps);
	}
	{
		int steps = 0;
		for (buffer::const_iterator p = b; p != e; ++p, ++steps) {}
		EXPECT_EQ(INSERT_CHARS, steps);
	}

	{
		buffer out1;
		EXPECT_DEATH({ out.begin() - out1.begin(); }, "Iterator belongs to container");
	}
}

TEST(OutgoingBuffer, DISABLED_ReverseIterators)
{
	buffer out;
	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	EXPECT_EQ(INSERT_CHARS, out.size());
	int steps = 0;
	for (buffer::reverse_iterator p = out.rbegin(); p != out.rend(); ++p, ++steps) {}
	EXPECT_EQ(INSERT_CHARS, steps);
}

TEST(OutgoingBuffer, Encapsulation)
{
	buffer out;
	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	EXPECT_EQ(INSERT_CHARS, out.size());
	{
		buffer::encapsulation encaps(out.begin_encapsulation());
		for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
			out.push_back(i);
		}
		EXPECT_EQ(INSERT_CHARS, encaps.size());
		EXPECT_EQ(INSERT_CHARS*2, out.size()); // Before encapsulation is closed
	}
	EXPECT_EQ(INSERT_CHARS*2 + 3, out.size()); // After encapsulation is closed, size of 100 fits into one byte, 2 bytes are encaps header
}

TEST(OutgoingBuffer, NestedEncapsulation)
{
	const size_t INNER_ENCAPS_HEADER = 3;
	const size_t OUTER_ENCAPS_HEADER = 4;
	buffer out;
	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	EXPECT_EQ(INSERT_CHARS, out.size());
	{
		buffer::encapsulation outer(out.begin_encapsulation());
		for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
			out.push_back(i);
		}
		EXPECT_EQ(INSERT_CHARS, outer.size());
		EXPECT_EQ(INSERT_CHARS*2, out.size()); // Before encapsulation is closed
		{
			buffer::encapsulation inner(out.begin_encapsulation());
			for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
				out.push_back(i);
			}
			EXPECT_EQ(INSERT_CHARS, inner.size());
			EXPECT_EQ(INSERT_CHARS*2, outer.size());
			EXPECT_EQ(INSERT_CHARS*3, out.size()); // Before encapsulation is closed
		}
		EXPECT_EQ(INSERT_CHARS*2 + INNER_ENCAPS_HEADER, outer.size());
		EXPECT_EQ(INSERT_CHARS*3 + INNER_ENCAPS_HEADER, out.size()); // Before encapsulation is closed
		for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
			out.push_back(i);
		}
		EXPECT_EQ(INSERT_CHARS*3 + INNER_ENCAPS_HEADER, outer.size());
		EXPECT_EQ(INSERT_CHARS*4 + INNER_ENCAPS_HEADER, out.size()); // Before encapsulation is closed

		buffer opaque;
		for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
			opaque.push_back(i);
		}
		EXPECT_EQ(INSERT_CHARS, opaque.size());
		out.insert_encapsulation(std::move(opaque));

		EXPECT_EQ(INSERT_CHARS*4 + INNER_ENCAPS_HEADER*2, outer.size());
		EXPECT_EQ(INSERT_CHARS*5 + INNER_ENCAPS_HEADER*2, out.size()); // Before encapsulation is closed
	}
	EXPECT_EQ(INSERT_CHARS*5 + INNER_ENCAPS_HEADER*2 + OUTER_ENCAPS_HEADER, out.size()); // After encapsulation is closed, size of 200 fits into two bytes, 2 bytes per encaps header
}

}  // namespace test
}  // namespace encoding
}  // namespace wire

