/*
 * varint_io_test.cpp
 *
 *  Created on: May 4, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/wire_io.hpp>
#include <wire/encoding/buffers.hpp>

namespace wire {
namespace encoding {
namespace test {

enum some_enum {
    value_none  = 0,
    value_one   = 0x01,
    value_two   = 0x02
};

TEST(IO, Varint)
{
    outgoing out;
    {
        outgoing::encaps_guard encaps{ out.begin_encapsulation() };
        auto o = ::std::back_inserter(out);
        write(o, 100500UL);
        write(o, value_one);
        write(o, static_cast<some_enum>(value_one | value_two));
        write(o, value_two);
        write(o, 42);
        EXPECT_FALSE(encaps.empty());
    }
    incoming in{ message{}, ::std::move(out) };
    {
        incoming::encaps_guard encaps{ in.begin_encapsulation(in.begin()) };
        EXPECT_FALSE(encaps.empty());

        auto f = encaps->begin();
        auto l = encaps->end();

        ::std::size_t sz;
        EXPECT_NO_THROW(read(f, l, sz));
        EXPECT_EQ(100500UL, sz);
        some_enum e;
        EXPECT_NO_THROW(read(f, l, e));
        EXPECT_EQ(value_one, e);
        EXPECT_NO_THROW(read(f, l, e));
        EXPECT_EQ(value_one | value_two, e);
        EXPECT_NO_THROW(read(f, l, e));
        EXPECT_EQ(value_two, e);
        int i;
        EXPECT_NO_THROW(read(f, l, i));
        EXPECT_EQ(42, i);
    }
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */
