/*
 * classes_io.cpp
 *
 *  Created on: May 11, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <test/classes_for_io.hpp>
#include <wire/encoding/buffers.hpp>

namespace wire {
namespace encoding {
namespace test {

TEST(IO, Polymorphic)
{
    outgoing out{ core::connector_ptr{} };
    {
        ::test::derived_ptr inst1 = ::std::make_shared< ::test::derived >();
        inst1->fvalue = 3.14;
        inst1->svalue = "inst1";
        inst1->ivalue = 100500;

        ::test::derived_ptr inst2 = ::std::make_shared< ::test::derived >();
        inst2->fvalue = 2.718;
        inst2->svalue = "inst2";
        inst2->bvalue = inst1;
        inst2->ivalue = 42;

        ::test::derived_ptr inst3 = ::std::make_shared< ::test::derived >();
        inst3->fvalue = 2.718 * 3.14;
        inst3->svalue = "inst3";
        inst3->bvalue = inst1;
        inst3->ivalue = 13;

        ::test::base_ptr b1 = inst2;
        ::test::base_ptr b2 = inst3;

        write(::std::back_inserter(out), b1, b2);
        out.close_all_encaps();
        ::std::cerr << "Out buffer size " << out.size() << "\n";
        out.debug_print(::std::cerr);
    }

    incoming in{ message{}, ::std::move(out) };
    {
        ::test::base_ptr b1;
        ::test::base_ptr b2;

        auto encaps = in.current_encapsulation();
        auto b = encaps.begin();
        auto e = encaps.end();
        read(b, e, b1, b2);
        encaps.read_indirection_table(b);

        ASSERT_TRUE(b1.get());
        ASSERT_TRUE(b2.get());

        auto d1 = ::std::dynamic_pointer_cast<::test::derived>(b1);
        auto d2 = ::std::dynamic_pointer_cast<::test::derived>(b2);

        ASSERT_TRUE(d1.get());
        ASSERT_TRUE(d2.get());
        EXPECT_TRUE(d1->bvalue.get());

        EXPECT_EQ(d1->bvalue, d2->bvalue);
    }
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */

