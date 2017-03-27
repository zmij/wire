/*
 * io_service_wait_thread.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_THREAD_HPP_
#define WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_THREAD_HPP_

#include <wire/asio_config.hpp>
#include <thread>

namespace wire {
namespace util {

template < typename Pred >
void
run_while( asio_config::io_service_ptr svc, Pred pred )
{
    ::std::thread t{
    [svc, pred](){
        while(pred())
            svc->poll();
    }};
    t.join();
}

template < typename Pred >
void
run_until( asio_config::io_service_ptr svc, Pred pred)
{
    ::std::thread t{
    [svc, pred](){
        while(!pred())
            svc->poll();
    }};
    t.join();
}

}  /* namespace util */
}  /* namespace wire */


#endif /* WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_THREAD_HPP_ */
