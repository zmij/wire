/*
 * exception_io_test.cpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/errors/user_exception.hpp>
#include <wire/errors/unexpected.hpp>
#include <wire/encoding/buffers.hpp>
#include <wire/util/murmur_hash.hpp>


namespace wire {
namespace encoding {
namespace test {

using namespace std::string_literals;

namespace {

::std::string const DERIVED = "::test::derived_type";
::std::string const BASE =  "::test::base_type";

}  /* namespace  */

class base : public errors::user_exception {
public:
    using ptr = ::std::shared_ptr<base>;
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
protected:
    void
    stream_message(::std::ostream& os) const override
    {
        os << "base exception";
    }
private:
    using factory_init_type = errors::user_exception_factory_init< base >;
    static factory_init_type _factory_init_;
};

base::factory_init_type base::_factory_init_;

class derived : public base {
public:
    using ptr = ::std::shared_ptr<derived>;
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

protected:
    void
    stream_message(::std::ostream& os) const override
    {
        os << "derived exception " << some_int;
    }

private:
    using factory_init_type = errors::user_exception_factory_init<derived>;
    static factory_init_type _factory_init_;
};

derived::factory_init_type derived::_factory_init_;

}

namespace detail {

template <>
struct is_user_exception<test::base> : ::std::true_type {};
template <>
struct is_user_exception<test::derived> : ::std::true_type {};

} /* namespace detail */


namespace test {

static_assert( detail::is_user_exception<base>::value,
        "Traits for base object are correct" );
static_assert( detail::is_user_exception<derived>::value,
        "Traits for derived object are correct" );

static_assert( detail::wire_type< base::ptr >::value == detail::wire_types::EXCEPTION,
        "Correct wire type for base object");
static_assert( detail::wire_type< derived::ptr >::value == detail::wire_types::EXCEPTION,
        "Correct wire type for derived object");

static_assert( ::std::is_base_of< detail::exception_reader<base>, detail::reader<base::ptr> >::value,
        "Correct reader for base object");
static_assert( ::std::is_base_of< detail::exception_reader<derived>, detail::reader<derived::ptr> >::value,
        "Correct reader for base object");

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
        EXPECT_NO_THROW(e.post_unmarshal());
        ::std::cerr << "Data left " << (encaps.end() - f) << "\n";
        EXPECT_EQ(::std::string{"base exception"}, e.what());
        derived d;
        l = encaps.end();
        EXPECT_NO_THROW(d.__wire_read(f, l, true));
        EXPECT_NO_THROW(d.post_unmarshal());
        ::std::cerr << "Data left " << (encaps.end() - f) << "\n";
        EXPECT_EQ(100500, d.some_int);
        EXPECT_EQ(::std::string{"derived exception 100500"}, d.what());
    }
}

TEST(IO, UserExceptionFactory)
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
    incoming in{ message{}, ::std::move(out) };
    {
        auto encaps = in.current_encapsulation();
        EXPECT_FALSE(encaps.empty());

        auto f = encaps.begin();
        auto l = encaps.end();
        ::std::cerr << "Encaps size " << encaps.size() << "\n";
        ::std::cerr << "Data left " << (encaps.end() - f) << "\n";

        errors::user_exception_ptr e, d;
        read(f, l, e);
        read(f, l, d);
        encaps.read_indirection_table(f);

        ASSERT_TRUE(e.get());
        ASSERT_TRUE(d.get());

        auto bs = ::std::dynamic_pointer_cast< base >(e);
        EXPECT_TRUE(bs.get()) << "Correct type info";
        auto dv = ::std::dynamic_pointer_cast< derived >(e);
        EXPECT_FALSE(dv.get()) << "Correct type info";
        dv = ::std::dynamic_pointer_cast< derived >(d);
        EXPECT_TRUE(dv.get()) << "Correct type info";

        EXPECT_EQ(::std::string{"base exception"}, e->what());
        EXPECT_EQ(::std::string{"derived exception 100500"}, d->what());

        auto ex = e->make_exception_ptr();
        EXPECT_THROW(::std::rethrow_exception(ex), base);
        ex = d->make_exception_ptr();
        EXPECT_THROW(::std::rethrow_exception(ex), derived);
    }
}

TEST(IO, UnexpectedException)
{
    outgoing out{ core::connector_ptr{} };
    {
        auto encaps = out.current_encapsulation();
        auto o = ::std::back_inserter(out);
        errors::unexpected e{ "some_type"s, "message"s };
        EXPECT_EQ("some_type", e.type_name);
        EXPECT_EQ("message", e.message);
        e.__wire_write(o);
        ::std::cerr << "Encaps size " << encaps.size() << "\n";
        EXPECT_FALSE(encaps.empty());
        out.debug_print(::std::cerr);
    }
    incoming in{ message{}, ::std::move(out) };
    {
        auto encaps = in.current_encapsulation();
        EXPECT_FALSE(encaps.empty());

        auto f = encaps.begin();
        auto l = encaps.end();
        ::std::cerr << "Encaps size " << encaps.size() << "\n";
        ::std::cerr << "Data left " << (encaps.end() - f) << "\n";

        errors::user_exception_ptr e;
        read(f, l, e);
        encaps.read_indirection_table(f);

        ASSERT_TRUE(e.get()) << "Exception pointer read from stream";
        auto u = ::std::dynamic_pointer_cast< errors::unexpected >(e);
        ASSERT_TRUE(u.get()) << "Correct exception runtime type";

        EXPECT_EQ("message", u->message);
        EXPECT_EQ("some_type", u->type_name);
    }
}

}  /* namespace test */
}  /* namespace encoding */
}  /* namespace wire */
