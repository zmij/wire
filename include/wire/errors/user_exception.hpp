/*
 * user_exception.hpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_ERRORS_USER_EXCEPTION_HPP_
#define WIRE_ERRORS_USER_EXCEPTION_HPP_

#include <wire/types.hpp>
#include <wire/errors/exceptions.hpp>
#include <wire/encoding/buffers.hpp>
#include <wire/encoding/segment.hpp>
#include <wire/encoding/detail/polymorphic_io.hpp>

namespace wire {
namespace errors {

class user_exception : public runtime_error {
public:
    using input_iterator = encoding::incoming::const_iterator;
    using output_iterator = ::std::back_insert_iterator<encoding::outgoing>;
public:
    user_exception() : runtime_error{""} {}
    explicit
    user_exception(std::string const& msg) : runtime_error{msg} {}

    template < typename ... T >
    user_exception(T const& ... args)
        : runtime_error(util::delim_concatenate(" ", args ...)) {}

    virtual ~user_exception() {}


    virtual void
    __wire_write(output_iterator out) const = 0;

    virtual void
    __wire_read(input_iterator& begin, input_iterator end, bool read_head = true) = 0;

    void
    post_unmarshal();

    virtual ::std::exception_ptr
    make_exception_ptr() = 0;

public:
    static ::std::string const&
    wire_static_type_id();
};

using user_exception_ptr = ::std::shared_ptr< user_exception >;

using user_exception_factory = encoding::detail::object_factory<user_exception>;

template < typename T >
using user_exception_factory_init = encoding::detail::object_factory_init<user_exception, T>;

}  /* namespace errors */
namespace encoding {
namespace detail {

template < typename T >
struct writer_impl< T, EXCEPTION > {
    using exception_value       = typename polymorphic_type<T>::type;
    using exception_ptr         = ::std::shared_ptr<exception_value const>;
    using exception_weak_ptr    = ::std::weak_ptr<exception_value const>;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, exception_value const& ex)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;
        ex.__wire_write(o);
    }

    template < typename OutputIterator >
    static void
    output(OutputIterator o, exception_ptr ex)
    {
        using output_iterator_check = octet_output_iterator_concept< OutputIterator >;

        if (ex) {
            ex->__wire_write(o);
        } else {
            throw errors::marshal_error{ "Cannot marshal an empty exception pointer" };
        }
    }
    template < typename OutputIterator >
    static void
    output(OutputIterator o, exception_weak_ptr ex)
    {
        output(o, ex.lock());
    }
};

template < typename T >
struct exception_reader {
    using exception_value       = T;
    using exception_ptr         = ::std::shared_ptr<exception_value>;
    using exception_weak_ptr    = ::std::weak_ptr<exception_value>;

    using input_iterator        = encoding::incoming::const_iterator;

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, exception_value& v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;

        v.__wire_read(begin, end);
        v.post_unmarshal();
    }

    static void
    input(input_iterator& begin, input_iterator end, exception_ptr& v)
    {
        using errors::user_exception_factory;
        auto tmp = user_exception_factory::instance().read< exception_value >(begin, end);
        ::std::swap(tmp, v);
    }
};

template < typename T >
struct reader_impl< T, EXCEPTION >
    : exception_reader< typename polymorphic_type<T>::type > {};

template <>
struct is_user_exception<errors::user_exception> : ::std::true_type{};

}  /* namespace detail */
}  /* namespace encoding */
}  /* namespace wire */

#endif /* WIRE_ERRORS_USER_EXCEPTION_HPP_ */
