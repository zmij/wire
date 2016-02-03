/*
 * object.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_OBJECT_HPP_
#define WIRE_CORE_OBJECT_HPP_

#include <wire/core/object_fwd.hpp>
#include <wire/core/current.hpp>
#include <wire/core/dispatch_request_fwd.hpp>
#include <unordered_map>

namespace wire {
namespace core {

class dispatcher_object {
public:
	virtual
	~dispatcher_object() = default;

	virtual void
	__dispatch(dispatch_request const&, current const&) = 0;
};

class object : public dispatcher_object {
public:
	virtual
	~object() = default;

	virtual bool
	wire_is_a(std::string const&, current const& = no_current) const;
	void
	__wire_is_a(dispatch_request const&, current const&);

	virtual void
	wire_ping(current const& = no_current) const;
	void
	__wire_ping(dispatch_request const&, current const&);


	virtual void
	__dispatch(dispatch_request const&, current const&);
private:
};

}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_OBJECT_HPP_ */
