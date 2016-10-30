/*
 * exception_io_test.cpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/errors/user_exception.hpp>
#include <wire/encoding/buffers.hpp>
#include <wire/util/murmur_hash.hpp>


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
    wire_static_type_id()
    {
        static ::std::string _type_id = BASE;
        return _type_id;
    }

    static hash_value_type
    wire_static_type_id_hash()
    {
        static hash_value_type _hash = hash::murmur_hash_64(BASE);
        return _hash;
    }

    void
    __wire_write(output_iterator out) const override
    {
        auto encaps = out.encapsulation();
        encaps.start_segment(wire_static_type_id(), ::wire::encoding::segment_header::last_segment);

        encaps.end_segment();
    }

    void
    __wire_read(input_iterator& begin, input_iterator end, bool read_head = true) override
    {
        auto encaps = begin.incoming_encapsulation();
        if (read_head) {
            ::wire::encoding::segment_header sh;
            encaps.read_segment_header(begin, end, sh);
            check_segment_header< base >(sh);
        }
    }

    ::std::exception_ptr
    make_exception_ptr() override
    { return ::std::make_exception_ptr(*this); }
};

class derived : public base {
public:
    derived() : base{}, some_int{} {};

    derived(::std::int32_t i) : base(wire_static_type_id(), i), some_int{i} {}

    static ::std::string const&
    wire_static_type_id()
    {
        static ::std::string _type_id = DERIVED;
        return _type_id;
    }

    static hash_value_type
    wire_static_type_id_hash()
    {
        static hash_value_type _hash = hash::murmur_hash_64(DERIVED);
        return _hash;
    }


    void
    __wire_write(output_iterator out) const override
    {
        auto encaps = out.encapsulation();
        encaps.start_segment(wire_static_type_id_hash(), ::wire::encoding::segment_header::none);
        write(out, some_int);
        encaps.end_segment();
        base::__wire_write(out);
    }

    void
    __wire_read(input_iterator& begin, input_iterator end, bool read_head = true) override
    {
        auto encaps = begin.incoming_encapsulation();
        if (read_head) {
            ::wire::encoding::segment_header sh;
            encaps.read_segment_header(begin, end, sh);
            check_segment_header< derived >(sh);
        }
        read(begin, end, some_int);
        base::__wire_read(begin, encaps.end(), true);
    }

    ::std::exception_ptr
    make_exception_ptr() override
    { return ::std::make_exception_ptr(*this); }
public:
    ::std::int32_t      some_int;
};

TEST(IO, UserException)
{
    outgoing out{ core::connector_ptr{} };
    {
        auto encaps = out.current_encapsulation();
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
        auto encaps = in.current_encapsulation();
        EXPECT_FALSE(encaps.empty());

        base e;
        auto f = encaps.begin();
        auto l = encaps.end();
        ::std::cerr << "Encaps size " << encaps.size() << "\n";
        ::std::cerr << "Data left " << (encaps.end() - f) << "\n";
        EXPECT_NO_THROW(e.__wire_read(f, l, true));
        ::std::cerr << "Data left " << (encaps.end() - f) << "\n";
        derived d;
        l = encaps.end();
        EXPECT_NO_THROW(d.__wire_read(f, l, true));
        ::std::cerr << "Data left " << (encaps.end() - f) << "\n";
        EXPECT_EQ(100500, d.some_int);
    }
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */
