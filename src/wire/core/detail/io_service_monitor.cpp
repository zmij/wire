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

asio_ns::io_service::id io_service_monitor::id;

struct io_service_monitor::impl {
    functional::callback_set<>  callbacks_;
};

io_service_monitor::io_service_monitor(asio_ns::io_service& owner)
    : asio_ns::io_service::service{owner}, pimpl_{ new impl{} }
{
    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Create io_service_monitor\n";
    #endif
}

io_service_monitor::~io_service_monitor()
{
    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Delete io_service_monotor\n";
    #endif
}

void
io_service_monitor::shutdown_service()
{
    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Shutdown IO service\n";
    #endif
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
