/*
 * io_service_wait_thread.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_THREAD_HPP_
#define WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_THREAD_HPP_

#include <wire/asio_config.hpp>
#include <wire/util/debug_log.hpp>
#include <thread>

namespace wire {
namespace util {


template < typename Pred >
void
run_while( asio_config::io_service_ptr svc, Pred pred )
{
    asio_config::io_service::work w(*svc);
    ::std::thread t{
    [svc, pred](){
        DEBUG_LOG(3, "=== Begin poll io_service (while condition)");
        while(pred()) svc->poll();
        DEBUG_LOG(3, "=== End poll io_service (while condition)");
    }};
    DEBUG_LOG(3, "*** Start wait done (while condition)");
    t.join();
    DEBUG_LOG(3, "*** End wait done (while condition)");
}

template < typename Pred >
void
run_until( asio_config::io_service_ptr svc, Pred pred)
{
    asio_config::io_service::work w(*svc);
    ::std::thread t{
    [svc, pred](){
        DEBUG_LOG(3, "=== Begin poll io_service (until condition)");
        while(!pred()) svc->poll();
        DEBUG_LOG(3, "=== End poll io_service (until condition)");
    }};
    DEBUG_LOG(3, "*** Start wait done (until condition)");
    t.join();
    DEBUG_LOG(3, "*** End wait done (until condition)");
}

}  /* namespace util */
}  /* namespace wire */


#endif /* WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_THREAD_HPP_ */
