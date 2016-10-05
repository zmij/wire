/*
 * service_runner.hpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_SERVICE_RUNNER_HPP_
#define WIRE_UTIL_SERVICE_RUNNER_HPP_

#include <wire/asio_config.hpp>
#include <memory>
#include <functional>

namespace wire {
namespace util {

class service_runner {
public:
    using thread_func   = ::std::function<void()>;
    using stop_func     = ::std::function<void()>;
public:
    service_runner(asio_config::io_service_ptr,
        stop_func sfunc = nullptr, thread_func tfunc = nullptr);
    ~service_runner();

    void
    run(::std::size_t threads);
    void
    stop();
private:
private:
    asio_config::io_service_ptr io_svc_;
    ASIO_NS::signal_set         signals_;

    thread_func                 run_;
    stop_func                   stop_;
};

}  /* namespace util */
}  /* namespace wire */


#endif /* WIRE_UTIL_SERVICE_RUNNER_HPP_ */
