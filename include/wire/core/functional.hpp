/*
 * callbacks.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_FUNCTIONAL_HPP_
#define WIRE_CORE_FUNCTIONAL_HPP_

#include <functional>
#include <exception>
#include <vector>
#include <wire/asio_config.hpp>

namespace wire {
namespace core {
namespace functional {

using void_callback         = ::std::function< void() >;

template < typename ... T >
using callback              = ::std::function< void (T ... ) >;

using exception_callback    = callback< ::std::exception_ptr >;

using void_result_pair      = ::std::pair< void_callback, exception_callback >;

template < typename ... T >
using result_pair           = ::std::pair< callback< T... >, exception_callback >;

template < typename ... T >
struct callback_set {
    using callback_type    = callback< T ... >;
    void
    operator()(T&& ... args) const
    {
        for (auto cb : callbacks_) {
            try {
                cb(::std::forward<T>(args)...);
            } catch (...) {
            }
        }
    }
    void
    add_callback(callback_type cb)
    {
        callbacks_.push_back(cb);
    }
private:
    ::std::vector< callback_type > callbacks_;
};

template <>
struct callback_set<> {
    using callback_type    = void_callback;
    void
    operator()() const
    {
        for (auto cb : callbacks_) {
            try {
                cb();
            } catch (...) {
            }
        }
    }
    void
    add_callback(callback_type cb)
    {
        callbacks_.push_back(cb);
    }
private:
    ::std::vector< callback_type > callbacks_;
};

inline void
report_exception(exception_callback handler, ::std::exception_ptr ex) noexcept
{
    if (handler) {
        try {
            handler(ex);
        } catch(...) {}
    }
}

inline void
report_exception(exception_callback handler, asio_config::error_code const& ec) noexcept
{
    if (handler) {
        try {
            auto ex = ::std::make_exception_ptr( asio_config::system_error{ ec } );
            handler( ex );
        } catch (...) {}
    }
}

template < typename Exception >
void
report_exception(exception_callback handler, Exception&& err) noexcept
{
    if (handler) {
        try {
            auto ex = ::std::make_exception_ptr(::std::forward<Exception>(err));
            handler( ex );
        } catch(...) {}
    }
}

template < typename Handler, typename Result >
void
report_async_result(Handler handler, Result&& res)
{
    if (handler) {
        try {
            handler(::std::forward<Result>(res));
        } catch(...) {}
    }
}


}  // namespace functional
}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_FUNCTIONAL_HPP_ */
