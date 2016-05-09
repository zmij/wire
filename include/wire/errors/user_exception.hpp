/*
 * user_exception.hpp
 *
 *  Created on: May 3, 2016
 *      Author: zmij
 */

#ifndef WIRE_ERRORS_USER_EXCEPTION_HPP_
#define WIRE_ERRORS_USER_EXCEPTION_HPP_

#include <wire/errors/exceptions.hpp>
#include <wire/encoding/buffers.hpp>
#include <wire/encoding/segment.hpp>

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

    virtual ::std::exception_ptr
    make_exception_ptr() = 0;

    template < typename T >
    void
    __check_segment_header(encoding::segment_header const& seg_head)
    {
        switch (seg_head.type_id.which()) {
            case 0: {
                if (::boost::get< ::std::string >(seg_head.type_id) != T::wire_static_type_id()) {
                    throw unmarshal_error("Incorrect type id ", seg_head.type_id,
                            " expected ", T::wire_static_type_id());
                }
                break;
            }
            case 1:
                if (::boost::get< ::std::uint64_t >(seg_head.type_id) != T::wire_static_type_id_hash()) {
                    throw unmarshal_error("Incorrect type id ", seg_head.type_id,
                            " expected ", T::wire_static_type_id_hash());
                }
                break;
            default:
                throw unmarshal_error("Unexpected data type in segment type id");
        }
    }
};

using user_exception_ptr = ::std::shared_ptr< user_exception >;

class user_exception_factory {
public:
    using factory_function = ::std::function<::std::shared_ptr<user_exception>()>;

    static user_exception_factory&
    instance();

    void
    add_factory(::std::string const& type_id, ::std::uint64_t type_id_hash,
            factory_function func);

    factory_function
    get_factory(::std::string const& type_id) const;
    factory_function
    get_factory(::std::uint64_t type_id_hash) const;
    factory_function
    get_factory(encoding::segment_header::type_id_type const&) const;

    bool
    has_factory(::std::string const& type_id) const;
    bool
    has_factory(::std::uint64_t type_id_hash) const;
    bool
    has_factory(encoding::segment_header::type_id_type const&) const;

    template < typename T >
    ::std::shared_ptr< T >
    create(encoding::segment_header::type_id_type const& id) const
    {
        static_assert( ::std::is_base_of<user_exception, T>::value,
                "Can create only instances of user_exception derived classes");
        auto func = get_factory(id);
        auto ex = func();
        if (!ex) {
            throw unmarshal_error{ "Failed to instantiate exception" };
        }

        auto derived = ::std::dynamic_pointer_cast< T >(ex);
        if (!derived)
            throw unmarshal_error{ "Failed to cast instantiated exception" };
        return derived;
    }
private:
    user_exception_factory() {};
    user_exception_factory(user_exception_factory const&) = delete;
    user_exception_factory(user_exception_factory&&) = delete;
    user_exception_factory&
    operator = (user_exception_factory const&) = delete;
    user_exception_factory&
    operator = (user_exception_factory&&) = delete;
private:
    using id_to_factory_map = ::std::unordered_map<::std::string, factory_function>;
    using hash_to_factory_map = ::std::unordered_map<::std::uint64_t, factory_function>;

    id_to_factory_map   id_to_factory_;
    hash_to_factory_map hash_to_factory_;
};

template < typename T >
struct user_exception_factory_init {
    user_exception_factory_init()
    {
        static_assert(::std::is_base_of<user_exception, T>::value,
                "user_exception_factory_init can be used to initialize user_exception factories only");
        user_exception_factory::instance().add_factory(
        T::wire_static_type_id(),
        T::wire_static_type_id_hash(),
        [](){
            return ::std::make_shared<T>();
        });
    }
};

}  /* namespace errors */
namespace encoding {
namespace detail {

template < typename T >
struct exception_type {
    using type = typename ::std::decay<T>::type;
};

template < typename T >
struct exception_type< ::std::shared_ptr< T > > {
    using type = typename ::std::decay<T>::type;
};

template < typename T >
struct exception_type< ::std::weak_ptr< T > > {
    using type = typename ::std::decay<T>::type;
};

template < typename T >
struct exception_type< ::std::unique_ptr< T > > {
    using type = typename ::std::decay<T>::type;
};

template < typename T >
struct exception_writer {
    using exception_type        = T;
    using exception_ptr         = ::std::shared_ptr<T const>;
    using exception_weak_ptr    = ::std::weak_ptr<T const>;

    template < typename OutputIterator >
    static void
    output(OutputIterator o, exception_type const& ex)
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
struct writer_impl< T, EXCEPTION >
    : exception_writer< typename exception_type<T>::type > {};

template < typename T >
struct exception_reader {
    using exception_type        = T;
    using exception_ptr         = ::std::shared_ptr<T>;
    using exception_weak_ptr    = ::std::weak_ptr<T>;

    using input_iterator        = encoding::incoming::const_iterator;

    template < typename InputIterator >
    static void
    input(InputIterator& begin, InputIterator end, exception_type& v)
    {
        using input_iterator_check = octet_input_iterator_concept< InputIterator >;

        v.__wire_read(begin, end);
    }

    static void
    input(input_iterator& begin, input_iterator end, exception_ptr& v)
    {
        using errors::user_exception_factory;

        auto encaps = begin.incoming_encapsulation();
        ::wire::encoding::segment_header seg_head;
        encaps.read_segment_header(begin, end, seg_head);

        while (!user_exception_factory::instance().has_factory(seg_head.type_id)) {
            // skip segment
            begin += seg_head.size;
            if (begin == end) {
                throw errors::unmarshal_error{ "Failed to read exception" };
            }
            encaps.read_segment_header(begin, end, seg_head);
        }

        auto tmp = user_exception_factory::instance().create< exception_type >(seg_head.type_id);
        tmp->__wire_read(begin, end, false);
        ::std::swap(tmp, v);
    }
};

template < typename T >
struct reader_impl< T, EXCEPTION >
    : exception_reader< typename exception_type<T>::type > {};

template <>
struct is_user_exception<errors::user_exception> : ::std::true_type{};

}  /* namespace detail */
}  /* namespace encoding */
}  /* namespace wire */

#endif /* WIRE_ERRORS_USER_EXCEPTION_HPP_ */
