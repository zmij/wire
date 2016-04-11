/*
 * ast.cpp
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/ast/ast.hpp>
#include <sstream>

namespace wire {
namespace ast {

//----------------------------------------------------------------------------
//	entity class implementation
//----------------------------------------------------------------------------
entity::entity()
	: scope_{}, name_{}
{
}


entity::entity(scope_ptr sc, ::std::string const& name)
	: scope_{sc}, name_{name}
{
	if (!sc) {
		throw std::runtime_error("Scope pointer is empty");
	}
	if (name.empty()) {
		throw std::runtime_error("Name is empty");
	}
}

void
entity::get_qualified_name(::std::ostream& os) const
{
	auto sc = scope_.lock();
	if (!sc && !name_.empty())
		throw ::std::runtime_error("Named entity with no scope");
	if (sc) {
		sc->get_qualified_name(os);
		os << "::" << name_;
	}
}

::std::string
entity::qualified_name() const
{
	::std::ostringstream os;
	get_qualified_name(os);
	return ::std::move(os.str());
}


//----------------------------------------------------------------------------
//	constant class implementation
//----------------------------------------------------------------------------
constant::constant(scope_ptr sc, ::std::string const& type,
			::std::string const& name, ::std::string const& literal)
	: entity{sc, name}, literal_{literal}
{
}

//----------------------------------------------------------------------------
//	name_space class implementation
//----------------------------------------------------------------------------
name_space&
name_space::global()
{
	static name_space gns_;
	return gns_;
}

void
name_space::clear_global()
{
	global().types_.clear();
	global().constants_.clear();
	global().nested_.clear();
}

}  // namespace ast
}  // namespace wire



