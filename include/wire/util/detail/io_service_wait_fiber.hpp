/*
 * io_service_wait_fiber.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_FIBER_HPP_
#define WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_FIBER_HPP_

#include <wire/future_config.hpp>
#include <wire/asio_config.hpp>

namespace wire {
namespace util {

template < typename Pred >
void
run_while( asio_config::io_service_ptr svc, Pred pred )
{
//    namespace this_fiber = ::boost::this_fiber;
//    fiber f{
//        [svc, pred](){
//            while(pred()) {
//                ::std::cerr << "Run while fiber\n";
//                svc->poll();
//                this_fiber::yield();
//            }
//        }};
//    f.join();
}

template < typename Pred >
void
run_until( asio_config::io_service_ptr svc, Pred pred)
{
//    namespace this_fiber = ::boost::this_fiber;
//    fiber f{
//        [svc, pred](){
//            while(!pred()) {
//                ::std::cerr << "Run until fiber\n";
//                svc->poll();
//                this_fiber::yield();
//            }
//        }};
//    f.join();
}

} /* namespace util */
} /* namespace wire */



#endif /* WIRE_UTIL_DETAIL_IO_SERVICE_WAIT_FIBER_HPP_ */
