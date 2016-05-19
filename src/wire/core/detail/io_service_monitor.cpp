/*
 * io_service_monitor.cpp
 *
 *  Created on: 19 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/core/detail/io_service_monitor.hpp>
#include <iostream>

namespace wire {
namespace core {
namespace detail {

ASIO_NS::io_service::id io_service_monitor::id;

struct io_service_monitor::impl {
    functional::callback_set<>  callbacks_;
};

io_service_monitor::io_service_monitor(ASIO_NS::io_service& owner)
    : ASIO_NS::io_service::service{owner}, pimpl_{ new impl{} }
{
    ::std::cerr << "Create io_service_monitor\n";
}

io_service_monitor::~io_service_monitor()
{
    ::std::cerr << "Delete io_service_monotor\n";
}

void
io_service_monitor::shutdown_service()
{
    ::std::cerr << "Shutdown IO service\n";
    pimpl_->callbacks_();
}

void
io_service_monitor::add_observer(functional::void_callback cb)
{
    pimpl_->callbacks_.add_callback(cb);
}

}  /* namespace detail */
}  /* namespace core */
}  /* namespace wire */
