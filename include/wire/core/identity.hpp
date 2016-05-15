/*
 * identity.hpp
 *
 *  Created on: 26 янв. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_IDENTITY_HPP_
#define WIRE_CORE_IDENTITY_HPP_

#include <string>
#include <iosfwd>
#include <boost/variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/optional.hpp>

#include <wire/encoding/detail/uuid_io.hpp>
#include <wire/encoding/detail/variant_io.hpp>
#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace core {

struct identity {
    using uuid_type = ::boost::uuids::uuid;
    using id_type   = ::boost::variant< ::std::string, uuid_type >;

    ::std::string  category;
    id_type        id;

    identity() : category{}, id{} {};
    identity(char const* s) : category{}, id{ ::std::string{s} } {}
    identity(::std::string const& id) : category{}, id{id} {}
    identity(uuid_type const& id) : category{}, id{id} {}
    identity(id_type const& id) : category{}, id{id} {}
    identity(::std::string const& category, ::std::string const& id)
        : category{category}, id{id} {}
    identity(::std::string const& category, uuid_type const& id)
        : category{category}, id{id} {}
    identity(::std::string const& category, id_type const& id)
        : category{category}, id{id} {}

    bool
    operator == (identity const& rhs) const
    {
        return category == rhs.category && id == rhs.id;
    }
    bool
    operator != (identity const& rhs) const
    {
        return !(*this == rhs);
    }

    bool
    operator < (identity const& rhs) const
    {
        return category < rhs.category || (category == rhs.category && id < rhs.id);
    }

    bool
    empty() const;

    static ::boost::uuids::random_generator&
    uuid_gen()
    {
        static ::boost::uuids::random_generator gen_;
        return gen_;
    }
    static identity
    random()
    {
        return identity{ uuid_gen()() };
    }
    static identity
    random(::std::string const& category)
    {
        return identity{ category, uuid_gen()() };
    }
};

using optional_identity = ::boost::optional< identity >;

template < typename OutputIterator >
void
wire_write(OutputIterator o, identity const& v)
{
    encoding::write(o, v.category, v.id);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, identity& v)
{
    encoding::read(begin, end, v.category, v.id);
}

::std::size_t
hash(identity const&);

::std::ostream&
operator << (::std::ostream& os, identity const& val);
::std::istream&
operator >> (::std::istream& is, identity& val);

}  // namespace core
}  // namespace wire

namespace std {

template <>
struct hash< ::wire::core::identity > {
    using argument_type = ::wire::core::identity;
    using result_type   = ::std::size_t;
    result_type
    operator()(argument_type const& v) const
    {
        return ::wire::core::hash(v);
    }
};

}  // namespace std


#endif /* WIRE_CORE_IDENTITY_HPP_ */
