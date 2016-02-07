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

#include <wire/encoding/detail/uuid_io.hpp>
#include <wire/encoding/detail/variant_io.hpp>
#include <wire/encoding/wire_io.hpp>

namespace wire {
namespace core {

struct identity {
	typedef boost::uuids::uuid							uuid_type;
	typedef boost::variant< std::string, uuid_type >	id_type;

	std::string category;
	id_type		id;

	identity() : category(), id() {};
	identity(std::string const& id) : category(), id(id) {}
	identity(std::string const& category, std::string const& id)
		: category(category), id(id) {}
	identity(uuid_type const& id) : category(), id(id) {}
	identity(std::string const& category, uuid_type const& id)
		: category(category), id(id) {}

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

	static boost::uuids::random_generator&
	uuid_gen()
	{
		static boost::uuids::random_generator gen_;
		return gen_;
	}
	static identity
	random()
	{
		return identity{ uuid_gen()() };
	}
	static identity
	random(std::string const& category)
	{
		return identity{ category, uuid_gen()() };
	}
};

template < typename OutputIterator >
void
wire_write(OutputIterator o, identity const& v)
{
	encoding::write(o, v.category);
	encoding::write(o, v.id);
}

template < typename InputIterator >
void
wire_read(InputIterator& begin, InputIterator end, identity& v)
{
	encoding::read(begin, end, v.category);
	encoding::read(begin, end, v.id);
}

::std::size_t
hash(identity const&);

}  // namespace core
}  // namespace wire

namespace std {

template <>
struct hash< ::wire::core::identity > {
	typedef ::wire::core::identity	argument_type;
	typedef ::std::size_t			result_type;
	result_type
	operator()(argument_type const& v) const
	{
		return ::wire::core::hash(v);
	}
};

}  // namespace std


#endif /* WIRE_CORE_IDENTITY_HPP_ */
