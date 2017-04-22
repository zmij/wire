/*
 * scheduled_task.hpp
 *
 *  Created on: Apr 21, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_SCHEDULED_TASK_HPP_
#define WIRE_UTIL_SCHEDULED_TASK_HPP_

#include <wire/asio_config.hpp>
#include <memory>
#include <functional>

namespace wire {
namespace util {

class scheduled_task : public ::std::enable_shared_from_this<scheduled_task> {
public:
    using timer_type    = ::asio_ns::system_timer;
    using duration_type = timer_type::duration;
    using time_point    = timer_type::time_point;

    using handler_type  = asio_config::asio_callback;
public:
    scheduled_task( asio_config::io_service_ptr io_svc,
            handler_type h)
        : timer_{ *io_svc }, handler_{ ::std::move(h) } {}

    void
    run_in( duration_type period )
    {
        auto _this = shared_from_this();
        timer_.expires_from_now(period);
        timer_.async_wait(
            [_this](asio_config::error_code const& ec)
            { _this->handler_(ec); });
    }

    void
    run_at(time_point abs_time)
    {
        auto _this = shared_from_this();
        timer_.expires_at(abs_time);
        timer_.async_wait(
            [_this](asio_config::error_code const& ec)
            { _this->handler_(ec); });
    }
private:
    ::asio_ns::system_timer     timer_;
    handler_type                handler_;
};

inline void
schedule_in( asio_config::io_service_ptr io_svc, asio_config::asio_callback handler,
        scheduled_task::duration_type period)
{
    auto task = ::std::make_shared< scheduled_task >(io_svc, handler);
    task->run_in(::std::move(period));
}

inline void
schedule_at( asio_config::io_service_ptr io_svc, asio_config::asio_callback handler,
        scheduled_task::time_point abs_time)
{
    auto task = ::std::make_shared< scheduled_task >(io_svc, handler);
    task->run_at(::std::move(abs_time));
}

} /* namespace util */
} /* namespace wire */

#endif /* WIRE_UTIL_SCHEDULED_TASK_HPP_ */
