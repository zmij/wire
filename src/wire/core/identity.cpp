/*
 * identity.cpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/identity.hpp>
#include <boost/uuid/uuid.hpp>

namespace wire {
namespace core {

namespace detail {
struct identity_hash_visitor : ::boost::static_visitor< ::std::size_t > {
	::std::size_t
	operator()(::std::string const& s) const
	{
		static ::std::hash<::std::string> str_hash;
		return str_hash(s);
	}
	::std::size_t
	operator()(::boost::uuids::uuid const& u) const
	{
		return hash_value(u);
	}
};
}  // namespace detail

::std::size_t
hash(identity const& v)
{
	detail::identity_hash_visitor hasher;
	::std::size_t id_hash = ::boost::apply_visitor(hasher, v.id) ^ (v.id.which() << 1);
	return hasher(v.category) ^ (id_hash << 1);
}

}  // namespace core
}  // namespace wire
