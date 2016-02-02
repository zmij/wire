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
	outgoing out;
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
	EXPECT_FALSE(out.empty());
}

TEST(OutgoingBuffer, ForwardIterators)
{
	outgoing out;
	outgoing::iterator b = out.begin();
	outgoing::iterator e = out.end();
	outgoing::const_iterator cb = out.cbegin();
	outgoing::const_iterator ce = out.cend();

	EXPECT_EQ(e, b);
	EXPECT_EQ(0, b - e);
	EXPECT_EQ(ce, cb);
	EXPECT_EQ(0, cb - ce);

	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	b = out.begin();
	e = out.end();
	EXPECT_EQ(INSERT_CHARS, out.size());
	EXPECT_FALSE(out.empty());
	ASSERT_NE(b, e);
	EXPECT_EQ(INSERT_CHARS, e - b);
	EXPECT_EQ(-INSERT_CHARS, b - e);
	{
		int steps = 0;
		for (outgoing::iterator p = b; p != e; ++p, ++steps);
		EXPECT_EQ(INSERT_CHARS, steps);
	}
	{
		int steps = 0;
		for (outgoing::const_iterator p = b; p != e; ++p, ++steps);
		EXPECT_EQ(INSERT_CHARS, steps);
	}
	{
		int steps = 0;
		for (outgoing::iterator p = e; p != b; --p, ++steps);
		EXPECT_EQ(INSERT_CHARS, steps);
	}

	{
		outgoing out1;
		EXPECT_DEATH({ out.begin() - out1.begin(); }, "Iterator belongs to container");
	}
}

TEST(OutgoingBuffer, ReverseIterators)
{
	outgoing out;
	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	EXPECT_EQ(INSERT_CHARS, out.size());
	int steps = 0;
	for (outgoing::reverse_iterator p = out.rbegin(); p != out.rend(); ++p, ++steps) {}
	EXPECT_EQ(INSERT_CHARS, steps);
}

TEST(OutgoingBuffer, Encapsulation)
{
	outgoing out;
	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	EXPECT_EQ(INSERT_CHARS, out.size());
	{
		outgoing::encapsulation encaps(out.begin_encapsulation());
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
	outgoing out;
	for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
		out.push_back(i);
	}
	EXPECT_EQ(INSERT_CHARS, out.size());
	{
		outgoing::encapsulation outer(out.begin_encapsulation());
		for (uint8_t i = 0; i < INSERT_CHARS; ++i) {
			out.push_back(i);
		}
		EXPECT_EQ(INSERT_CHARS, outer.size());
		EXPECT_EQ(INSERT_CHARS*2, out.size()); // Before encapsulation is closed
		{
			outgoing::encapsulation inner(out.begin_encapsulation());
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

		outgoing opaque;
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

