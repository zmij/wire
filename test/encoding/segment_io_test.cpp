/*
 * segment_io_test.cpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/encoding/buffers.hpp>

namespace wire {
namespace encoding {
namespace test {

namespace {

::std::string const DERIVED = "::test::derived_type";
::std::string const BASE =  "::test::base_type";

}  /* namespace  */

TEST(IO, Segment)
{
    outgoing out;
    {
        outgoing::encaps_guard encaps{ out.begin_encapsulation() };
        encaps->start_segment(BASE, segment_header::last_segment);
        encaps->start_segment(DERIVED);
        encaps->start_segment(BASE, segment_header::last_segment);
        encaps->start_segment(DERIVED);
        encaps->start_segment(BASE, segment_header::last_segment);
        encaps->start_segment(BASE, segment_header::last_segment);
        encaps->end_segment();
        EXPECT_FALSE(encaps.empty());
    }

    incoming in{ message{}, ::std::move(out) };
    {
        incoming::encaps_guard encaps{ in.begin_encapsulation(in.begin()) };
        EXPECT_FALSE(encaps.empty());

        auto f = encaps->begin();
        auto l = encaps->end();
        segment_header sh;
        EXPECT_NO_THROW(encaps->read_segment_header(f, l, sh));
        EXPECT_EQ(BASE, sh.type_id);
        EXPECT_TRUE(sh.flags & segment_header::string_type_id);

        l = encaps->end();
        EXPECT_NO_THROW(encaps->read_segment_header(f, l, sh));
        EXPECT_EQ(DERIVED, sh.type_id);
        EXPECT_TRUE(sh.flags & segment_header::string_type_id);

        l = encaps->end();
        EXPECT_NO_THROW(encaps->read_segment_header(f, l, sh));
        EXPECT_EQ(BASE, sh.type_id);
        EXPECT_FALSE(sh.flags & segment_header::string_type_id);

        l = encaps->end();
        EXPECT_NO_THROW(encaps->read_segment_header(f, l, sh));
        EXPECT_EQ(DERIVED, sh.type_id);
        EXPECT_FALSE(sh.flags & segment_header::string_type_id);

        l = encaps->end();
        EXPECT_NO_THROW(encaps->read_segment_header(f, l, sh));
        EXPECT_EQ(BASE, sh.type_id);
        EXPECT_FALSE(sh.flags & segment_header::string_type_id);

        l = encaps->end();
        EXPECT_NO_THROW(encaps->read_segment_header(f, l, sh));
        EXPECT_EQ(BASE, sh.type_id);
        EXPECT_FALSE(sh.flags & segment_header::string_type_id);
    }
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */
