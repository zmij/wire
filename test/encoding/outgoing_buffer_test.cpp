/*
 * ougoing_buffer_test.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/buffers.hpp>
#include <bitset>

namespace wire {
namespace encoding {
namespace test {

namespace {

const int INSERT_CHARS = 100;
const std::string LIPSUM_TEST_STRING = R"~(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque malesuada 
ut nulla vitae elementum. Curabitur dictum egestas mauris et accumsan. Aliquam 
erat volutpat. Proin tempor enim vitae purus hendrerit, id varius tellus 
malesuada. Phasellus mattis molestie est non auctor. Etiam porttitor est at 
commodo mollis. Sed ut imperdiet velit. Vivamus eget sapien in lorem consequat 
varius nec vitae eros. Suspendisse interdum arcu dui, eu placerat libero rutrum 
quis. Nullam molestie mattis rhoncus. Duis imperdiet, massa sit amet varius 
malesuada, turpis quam fringilla orci, in consequat lectus nisi non massa. 
Mauris nec purus aliquam massa pellentesque finibus quis sed nisi. Nunc ac 
sapien nulla. Duis volutpat dui vitae vestibulum molestie. Suspendisse sagittis 
quis ex vitae pulvinar. Phasellus hendrerit in erat non convallis.

Nulla facilisi. Nunc et enim sed tortor mollis varius. Cras quis bibendum 
sapien. Maecenas vitae lectus vel lorem gravida blandit. Vestibulum faucibus 
sed sapien at scelerisque. Curabitur sit amet purus varius, volutpat orci id, 
auctor libero. Fusce vulputate lectus sapien, eu ornare sem maximus quis. Sed 
pellentesque tempus porta. Nam consequat tincidunt molestie. Quisque mollis ut 
quam quis ultricies. Donec consectetur leo odio, a faucibus justo interdum et. 
Vivamus fermentum justo eu sapien lobortis, non luctus nisi sodales. Donec 
ullamcorper felis et justo consequat, a gravida magna interdum. Morbi at dictum 
nisl. Nam condimentum bibendum turpis, eget aliquam velit vulputate vitae. 
Aliquam non nibh et mi blandit varius ut nec quam.

In mattis, diam eget lobortis auctor, odio tellus tristique lectus, et sodales 
odio est ut ligula. Maecenas elit odio, fermentum et leo euismod, blandit 
vestibulum magna. Morbi non elementum quam. Nunc varius erat sed eros porttitor 
auctor. Praesent luctus orci rutrum, fermentum nisl sit amet, varius nisi. 
Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia 
Curae; Mauris id turpis eget ligula consequat tincidunt. Duis sed sem ante. Nunc 
fermentum risus nec orci dapibus accumsan. Maecenas vitae nunc lorem. Aenean 
maximus elementum maximus. Phasellus finibus mi dolor, in molestie sem ultrices 
ac.
)~";


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

TEST(OutgoingBuffer, MessageHeaders)
{
	outgoing out{message::request};
	request req{ 0, operation_specs{  core::identity::random(), "", "pewpew" },
			request::normal };
	EXPECT_NO_THROW(write(std::back_inserter(out), req));
	EXPECT_LT(0, out.size());
	{
		outgoing encaps;
		EXPECT_NO_THROW(write(std::back_inserter(encaps), LIPSUM_TEST_STRING));
		out.insert_encapsulation(std::move(encaps));
	}
	EXPECT_LT(0, out.size());
	std::size_t message_size = out.size();
	char data[4096];
	auto buffers = out.to_buffers();
	char* in = data;
	std::size_t header_size(0);
	for (auto const& buff : buffers) {
		char const* bdata = reinterpret_cast<char const*>(ASIO_NS::detail::buffer_cast_helper(buff));
		std::size_t sz = ASIO_NS::detail::buffer_size_helper(buff);
		if (!header_size) {
			header_size = sz;
		}
		std::cerr << "Buff size " << sz << "\n";
		in = std::copy(bdata, bdata + sz, in);
	}
	EXPECT_LT(out.size(), in - data);
	EXPECT_LT(0, header_size);
	message m;
	auto begin = data;
	auto end = in;
	EXPECT_NO_THROW(read(begin, end, m));
	EXPECT_EQ(message_size, m.size);
	EXPECT_EQ((end - begin), m.size);
	request req1;
	EXPECT_NO_THROW(read(begin, end, req1));
	EXPECT_EQ(req, req1);
}

}  // namespace test
}  // namespace encoding
}  // namespace wire

