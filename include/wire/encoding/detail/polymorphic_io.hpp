/*
 * polymorhic_io.hpp
 *
 *  Created on: May 10, 2016
 *      Author: zmij
 */

#ifndef WIRE_ENCODING_DETAIL_POLYMORPHIC_IO_HPP_
#define WIRE_ENCODING_DETAIL_POLYMORPHIC_IO_HPP_

#include <wire/encoding/detail/helpers.hpp>
#include <wire/encoding/detail/varint_io.hpp>
#include <wire/encoding/detail/wire_io_fwd.hpp>

#include <wire/encoding/segment.hpp>
#include <wire/encoding/buffers.hpp>

#include <wire/util/meta_helpers.hpp>

namespace wire {
namespace encoding {
namespace detail {

template < typename T >
class object_factory {
public:
    using root_type         = T;
    using object_ptr        = ::std::shared_ptr<root_type>;
    using factory_function  = ::std::function< object_ptr() >;
    using input_iterator    = incoming::const_iterator;

    static object_factory&
    instance();

    static ::std::string const&
    root_type_id();
    static hash_value_type
    root_type_id_hash();

    void
    add_factory(::std::string const& type_id, hash_value_type type_id_hash,
            factory_function func);

    factory_function
    get_factory(::std::string const& type_id) const;
    factory_function
    get_factory(hash_value_type type_id_hash) const;
    factory_function
    get_factory(segment_header::type_id_type const&) const;

    bool
    has_factory(::std::string const& type_id) const;
    bool
    has_factory(hash_value_type type_id_hash) const;
    bool
    has_factory(segment_header::type_id_type const&) const;

    ::std::string const&
    factory_name(segment_header::type_id_type const&) const;

    template < typename U >
    ::std::shared_ptr< U >
    read(input_iterator& begin, input_iterator end) const;
private:
    object_factory() {};
    object_factory(object_factory const&) = delete;
    object_factory(object_factory&&) = delete;
    object_factory&
    operator = (object_factory const&) = delete;
    object_factory&
    operator = (object_factory&&) = delete;
private:
    using id_to_factory_map = ::std::unordered_map< ::std::string, factory_function >;
    using hash_to_factory_map = ::std::unordered_map< hash_value_type, factory_function >;
    using factory_name_map = ::std::unordered_map< hash_value_type, ::std::string >;

    id_to_factory_map   id_to_factory_;
    hash_to_factory_map hash_to_factory_;
    factory_name_map    names_;
};

template < typename BaseType, typename T >
struct object_factory_init {
    using root_type = BaseType;
    using registry_type = object_factory<BaseType>;

    static_assert(::std::is_base_of<root_type, T>::value,
            "Can add factory only to appropriate factory registry");

    object_factory_init()
    {
        registry_type::instance().add_factory(
            T::wire_static_type_id(),
            T::wire_static_type_id_hash(),
            []()
            { return ::std::make_shared< T >(); }
        );
    }
};

template < typename T >
using auto_object_factory_init = object_factory_init< typename T::wire_root_type, T >;

template < typename T >
struct writer_impl< T, CLASS > {
    using class_type        = typename polymorphic_type<T>::type;
    using class_ptr         = ::std::shared_ptr<class_type>;
    using output_iterator   = outgoing::output_iterator;
    using object_stream_id  = outgoing::encapsulation_type::object_stream_id;

    static void
    output(output_iterator o, class_ptr val)
    {
        auto encaps = o.encapsulation();
        object_stream_id _id{0};
        if (val) {
            _id = encaps.enqueue_object(val,
                [o, val](object_stream_id id)
                {
                    ::std::cerr << "Output object " << class_type::wire_static_type_id()
                        << " with id " << id << "\n";
                    write(o, id);
                    if (!val) {
                        throw errors::marshal_error{ "Object pointer is empty :(" };
                    }
                    val->__wire_write(o);
                }
            );
        }
        write(o, -_id);
    }
};

template < typename T >
struct reader_impl < T, CLASS > {
    using class_type        = typename polymorphic_type<T>::type;
    using class_ptr         = ::std::shared_ptr<class_type>;
    using input_iterator    = incoming::const_iterator;
    using root_type         = typename class_type::wire_root_type;
    using root_ptr          = ::std::shared_ptr<class_type>;
    using factory_type      = object_factory<root_type>;

    static void
    input(input_iterator& begin, input_iterator end, class_ptr& p)
    {
        auto ref = ::std::ref(p);
        begin.incoming_encapsulation().read_object< root_type >(begin, end,
        [ref](root_ptr v)
        {
            ref.get() = ::std::dynamic_pointer_cast< class_type >(v);
            if (!ref.get().get()) {
                throw errors::unmarshal_error{ class_type::wire_static_type_id(),
                    " instance was expected" };
            }
        },
        [](input_iterator& b, input_iterator e)
        {
            return factory_type::instance().template read< class_type >(b, e);
        });
    }

};

}  /* namespace detail */
template < typename T >
void
check_segment_header(segment_header const& seg_head)
{
    switch (seg_head.type_id.which()) {
        case 0: {
            if (::boost::get< ::std::string >(seg_head.type_id) != T::wire_static_type_id()) {
                throw errors::unmarshal_error("Incorrect type id ", seg_head.type_id,
                        " expected ", T::wire_static_type_id());
            }
            break;
        }
        case 1:
            if (::boost::get< hash_value_type >(seg_head.type_id) != T::wire_static_type_id_hash()) {
                throw errors::unmarshal_error("Incorrect type id ", seg_head.type_id,
                        " expected ", T::wire_static_type_id_hash());
            }
            break;
        default:
            throw errors::unmarshal_error("Unexpected data type in segment type id");
    }
}

}  /* namespace encoding */
}  /* namespace wire */

#include <wire/encoding/detail/polymorphic_io.inl>

#endif /* WIRE_ENCODING_DETAIL_POLYMORPHIC_IO_HPP_ */
