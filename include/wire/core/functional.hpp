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

namespace wire {
namespace core {
namespace functional {

using void_callback         = ::std::function< void() >;

template < typename ... T >
using callback              = ::std::function< void (T ... ) >;

using exception_callback    = callback< ::std::exception_ptr >;

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

template < typename Exception >
void
report_exception(exception_callback handler, Exception&& err) noexcept
{
    if (handler) {
        try {
            handler(::std::make_exception_ptr(::std::forward<Exception>(err)));
        } catch(...) {}
    }
}


}  // namespace functional
}  // namespace core
}  // namespace wire



#endif /* WIRE_CORE_FUNCTIONAL_HPP_ */
