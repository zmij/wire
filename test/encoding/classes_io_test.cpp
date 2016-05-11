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

    outgoing out{ core::connector_ptr{} };
    write(::std::back_inserter(out), b1, b2);
    ::std::cerr << "Out buffer size " << out.size() << "\n";
    out.debug_print(::std::cerr);
    out.close_all_encaps();
    ::std::cerr << "Out buffer size " << out.size() << "\n";
    out.debug_print(::std::cerr);
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */

