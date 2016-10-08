/*
 * service_runner.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: zmij
 */

#include <wire/util/service_runner.hpp>
#include <iostream>

namespace wire {
namespace util {

service_runner::service_runner(asio_config::io_service_ptr io_svc,
        stop_func sfunc, thread_func tfunc)
    : io_svc_{io_svc},
      signals_{*io_svc_},
      run_{tfunc},
      stop_{sfunc}
{
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait([this](asio_config::error_code const&, int){ stop(); });
}

service_runner::~service_runner()
{
    stop();
}

void
service_runner::run(::std::size_t thread_count)
{
    if (thread_count > 1) {
        // run multithreaded
        ::std::vector<::std::thread> threads;
        threads.reserve(thread_count);

        auto promise = ::std::make_shared< ::std::promise<void> >();
        auto future = promise->get_future();
        auto tfunc =
            [this, promise](){
                try {
                    if (run_) {
                        run_();
                    } else {
                        io_svc_->run();
                    }
                    try {
                        promise->set_value();
                    } catch (...) {}
                } catch (...) {
                    try {
                        promise->set_exception(::std::current_exception());
                    } catch (...) {}
                    stop();
                }
            };

        for (::std::size_t t = 0; t < thread_count; ++t) {
            threads.emplace_back(tfunc);
        }

        for (auto& t : threads) {
            t.join();
        }
        #if DEBUG_OUTPUT >= 1
        ::std::cerr << "All treads exited\n";
        #endif
        future.get(); // will throw an error if a thread has set it
    } else {
        // run in single thread, propagate exception to outer scope
        try {
            if (run_) {
                run_();
            } else {
                io_svc_->run();
            }
        } catch (...) {
            stop();
            throw;
        }
    }
}

void
service_runner::stop()
{
    if (!io_svc_->stopped()) {
        if (stop_) {
            stop_();
        }
    }
    if (!io_svc_->stopped()) {
        io_svc_->stop();
    }
}

}  /* namespace util */
}  /* namespace wire */
