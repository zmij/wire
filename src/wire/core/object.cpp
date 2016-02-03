/*
 * object.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: zmij
 */

#include <wire/core/object.hpp>
#include <wire/core/dispatch_request.hpp>

namespace wire {
namespace core {

namespace {



}  // namespace

bool
object::wire_is_a(std::string const& id, current const&) const
{
	return false;
}

void
object::__wire_is_a(dispatch_request const& req, current const&)
{

}

void
object::wire_ping(current const&) const
{
}

void
object::__wire_ping(dispatch_request const& req, current const&)
{
}

void
object::__dispatch(dispatch_request const& req, current const& c)
{

}

}  // namespace core
}  // namespace wire
