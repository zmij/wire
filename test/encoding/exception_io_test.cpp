/*
 * exception_io_test.cpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/errors/user_exception.hpp>
#include <wire/encoding/buffers.hpp>


namespace wire {
namespace encoding {
namespace test {

namespace {

::std::string const DERIVED = "::test::derived_type";
::std::string const BASE =  "::test::base_type";

}  /* namespace  */

class base : public errors::user_exception {
public:
    base() : user_exception{} {}
    explicit
    base(::std::string const& msg) : user_exception{msg} {}

    template < typename ... T >
    base(T const& ... args) : user_exception(args ... ) {}

    static ::std::string const&
    static_type_id()
    {
        static ::std::string _type_id = BASE;
        return _type_id;
    }

    void
    __wire_write(output_iterator out) override
    {
        auto encaps = out.encapsulation();
        encaps.start_segment(static_type_id(), ::wire::encoding::segment_header::last_segment);

        encaps.end_segment();
    }

    void
    __wire_read(input_iterator& begin, input_iterator end, bool read_head) override
    {
        auto encaps = begin.incoming_encapsulation();
        if (read_head) {
            ::wire::encoding::segment_header sh;
            encaps.read_segment_header(begin, end, sh);
            if (sh.type_id != static_type_id()) {
                errors::unmarshal_error err(
                    "Incorrect type id ", sh.type_id,
                    " expected ", static_type_id());
                ::std::cerr << err.what() << "\n";
                throw err;
            }
        }
    }
};

class derived : public base {
public:
    derived() : base{}, some_int{} {};

    derived(::std::int32_t i) : base(static_type_id(), i), some_int{i} {}

    static ::std::string const&
    static_type_id()
    {
        static ::std::string _type_id = DERIVED;
        return _type_id;
    }

    void
    __wire_write(output_iterator out) override
    {
        auto encaps = out.encapsulation();
        encaps.start_segment(static_type_id(), ::wire::encoding::segment_header::none);
        write(out, some_int);
        encaps.end_segment();
        base::__wire_write(out);
    }

    void
    __wire_read(input_iterator& begin, input_iterator end, bool read_head) override
    {
        auto encaps = begin.incoming_encapsulation();
        if (read_head) {
            ::wire::encoding::segment_header sh;
            encaps.read_segment_header(begin, end, sh);
            if (sh.type_id != static_type_id()) {
                errors::unmarshal_error err(
                    "Incorrect type id ", sh.type_id,
                    " expected ", static_type_id());
                ::std::cerr << err.what() << "\n";
                throw err;
            }
        }
        read(begin, end, some_int);
        base::__wire_read(begin, encaps.end(), true);
    }
public:
    ::std::int32_t      some_int;
};

TEST(IO, UserException)
{
    outgoing out;
    {
        outgoing::encaps_guard encaps{ out.begin_encapsulation() };
        auto o = ::std::back_inserter(out);
        base e("ooops");
        e.__wire_write(o);
        ::std::cerr << "Encaps size " << encaps.size() << "\n";
        derived d{100500};
        d.__wire_write(o);
        ::std::cerr << "Encaps size " << encaps.size() << "\n";
        EXPECT_FALSE(encaps.empty());
    }
    ::std::cerr << "Out buffer size " << out.size() << "\n";
    out.debug_print(::std::cerr);

    incoming in{ message{}, ::std::move(out) };
    {
        incoming::encaps_guard encaps{ in.begin_encapsulation(in.begin()) };
        EXPECT_FALSE(encaps.empty());

        base e;
        auto f = encaps->begin();
        auto l = encaps->end();
        ::std::cerr << "Encaps size " << encaps->size() << "\n";
        ::std::cerr << "Data left " << (encaps->end() - f) << "\n";
        EXPECT_NO_THROW(e.__wire_read(f, l, true));
        ::std::cerr << "Data left " << (encaps->end() - f) << "\n";
        derived d;
        l = encaps->end();
        EXPECT_NO_THROW(d.__wire_read(f, l, true));
        ::std::cerr << "Data left " << (encaps->end() - f) << "\n";
        EXPECT_EQ(100500, d.some_int);
    }
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */
