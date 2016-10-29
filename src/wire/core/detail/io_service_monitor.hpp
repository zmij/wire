/*
 * io_service_monitor.hpp
 *
 *  Created on: 19 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_CORE_DETAIL_IO_SERVICE_MONITOR_HPP_
#define WIRE_CORE_DETAIL_IO_SERVICE_MONITOR_HPP_

#include <wire/asio_config.hpp>
#include <wire/core/functional.hpp>

#include <memory>

namespace wire {
namespace core {
namespace detail {

class io_service_monitor : public asio_ns::io_service::service {
public:
    static asio_ns::io_service::id id;
    io_service_monitor(asio_ns::io_service& owner);
    virtual ~io_service_monitor();

    void
    shutdown_service() override;

    void
    add_observer(functional::void_callback);
private:
    struct impl;
    typedef ::std::unique_ptr<impl> pimpl;
    pimpl pimpl_;
};

}  /* namespace detail */
}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_DETAIL_IO_SERVICE_MONITOR_HPP_ */
