/*
 * adapter.cpp
 *
 *  Created on: 7 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/adapter.hpp>
#include <wire/core/connection.hpp>
#include <unordered_map>

namespace wire {
namespace core {

struct adapter::impl {
	typedef ::std::unordered_map< endpoint, connection > connections;
	typedef ::std::unordered_map< identity, dispatcher_ptr > active_objects;
	typedef ::std::unordered_map< ::std::string, dispatcher_ptr > default_servants;

	asio_config::io_service_ptr io_service_;
	::std::string				name_;
	endpoints					endpoints_;

	connections					connections_;
	active_objects				active_objects_;
	default_servants			default_servants_;

	adapter_weak_ptr			owner_;

	impl(asio_config::io_service_ptr svc, ::std::string const& name)
		: io_service_(svc), name_(name)
	{
	}
	impl(asio_config::io_service_ptr svc, ::std::string const& name,
			endpoints&& eps)
		: io_service_(svc), name_(name), endpoints_{ std::move(eps) }
	{
	}
	impl(asio_config::io_service_ptr svc, ::std::string const& name,
			endpoints const& eps)
		: io_service_(svc), name_(name), endpoints_{ eps }
	{
	}

	void
	activate()
	{
		adapter_ptr adp = owner_.lock();
		if (adp) {
			if (endpoints_.empty()) {
				endpoints_.insert(endpoint::tcp("0.0.0.0", 0));
			}
			for (auto const& ep : endpoints_) {
				connections_.emplace(ep, std::move(connection{ adp, ep }));
			}
		} // TODO Throw an exception
	}

	void
	deactivate()
	{
		for (auto& c : connections_) {
			c.second.close();
		}
		connections_.clear();
	}

	void
	add_object(identity const& id, dispatcher_ptr disp)
	{
		active_objects_.insert(std::make_pair(id, disp));
	}

	void
	add_default_servant(std::string const& category, dispatcher_ptr disp)
	{
		default_servants_.insert(std::make_pair(category, disp));
	}

	dispatcher_ptr
	find_object(identity const& id)
	{
		auto o = active_objects_.find(id);
		if (o != active_objects_.end()) {
			return o->second;
		}
		auto d = default_servants_.find(id.category);
		if (d != default_servants_.end()) {
			return d->second;
		}
		d = default_servants_.find("");
		if (d != default_servants_.end()) {
			return d->second;
		}
		return dispatcher_ptr{};
	}
};

adapter_ptr
adapter::create_adapter(asio_config::io_service_ptr svc, ::std::string const& name)
{
	adapter_ptr a(new adapter{svc, name});
	a->pimpl_->owner_ = a;
	return a;
}

adapter_ptr
adapter::create_adapter(asio_config::io_service_ptr svc, ::std::string const& name,
		endpoint const& ep)
{
	adapter_ptr a(new adapter{svc, name, ep});
	a->pimpl_->owner_ = a;
	return a;
}

adapter_ptr
adapter::create_adapter(asio_config::io_service_ptr svc, ::std::string const& name,
		endpoints const& eps)
{
	adapter_ptr a(new adapter{svc, name, eps});
	a->pimpl_->owner_ = a;
	return a;
}

adapter::adapter(asio_config::io_service_ptr svc, ::std::string const& name)
	: pimpl_( ::std::make_shared<impl>(svc, name) )
{
}

adapter::adapter(asio_config::io_service_ptr svc, ::std::string const& name,
			endpoint const& ep)
	: pimpl_( ::std::make_shared<impl>( svc, name, std::move(endpoints{ ep }) ) )
{
}

adapter::adapter(asio_config::io_service_ptr svc, ::std::string const& name,
			endpoints const& eps)
	: pimpl_( ::std::make_shared<impl>(svc, name, eps) )
{
}

asio_config::io_service_ptr
adapter::io_service() const
{
	return pimpl_->io_service_;
}

void
adapter::activate()
{
	pimpl_->activate();
}

void
adapter::add_object(dispatcher_ptr disp)
{
	pimpl_->add_object(identity::random(), disp);
}

void
adapter::add_object(identity const& id, dispatcher_ptr disp)
{
	pimpl_->add_object(id, disp);
}

void
adapter::add_default_servant(dispatcher_ptr disp)
{
	pimpl_->add_default_servant("", disp);
}

void
adapter::add_default_servant(::std::string const& category, dispatcher_ptr disp)
{
	pimpl_->add_default_servant(category, disp);
}

dispatcher_ptr
adapter::find_object(identity const& id) const
{
	return pimpl_->find_object(id);
}

}  // namespace core
}  // namespace wire
