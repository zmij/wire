/*
 * observer_container.hpp
 *
 *  Created on: Nov 1, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_OBSERVER_CONTAINER_HPP_
#define WIRE_CORE_OBSERVER_CONTAINER_HPP_

#include <wire/asio_config.hpp>
#include <wire/core/connection_observer.hpp>

#include <boost/thread/shared_mutex.hpp>


namespace wire {
namespace core {
namespace detail {

struct observer_container : connection_observer {
    using mutex_type        = ::boost::shared_mutex;
    using shared_lock       = ::boost::shared_lock<mutex_type>;
    using exclusive_lock    = ::std::lock_guard<mutex_type>;

    observer_container(asio_config::io_service_ptr io_svc)
        : io_svc_{io_svc} {}
    observer_container(asio_config::io_service_ptr io_svc,
            connection_observer_set&& observers)
        : io_svc_{io_svc}, observers_{::std::move(observers)} {}
    virtual ~observer_container() {}

    void
    send_bytes(::std::size_t bytes, endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, bytes, ep_cp]()
                {
                    o->send_bytes(bytes, ep_cp);
                }
            );
        }
    }

    void
    receive_bytes(::std::size_t bytes, endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, bytes, ep_cp]()
                {
                    o->receive_bytes(bytes, ep_cp);
                }
            );
        }
    }

    void
    invoke_remote(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp]()
                {
                    o->invoke_remote(id, op, ep_cp);
                }
            );
        }
    }
    void
    invocation_ok(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp]()
                {
                    o->invocation_ok(id, op, ep_cp);
                }
            );
        }
    }
    void
    invocation_error(invocation_target const& id, operation_id const& op,
            endpoint const& ep, ::std::exception_ptr ex) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp, ex]()
                {
                    o->invocation_error(id, op, ep_cp, ex);
                }
            );
        }
    }

    void
    receive_request(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp]()
                {
                    o->receive_request(id, op, ep_cp);
                }
            );
        }
    }
    void
    request_ok(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp]()
                {
                    o->request_ok(id, op, ep_cp);
                }
            );
        }
    }
    void
    request_error(invocation_target const& id, operation_id const& op,
            endpoint const& ep, ::std::exception_ptr ex) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp, ex]()
                {
                    o->request_error(id, op, ep_cp, ex);
                }
            );
        }
    }
    void
    request_no_response(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp]()
                {
                    o->request_no_response(id, op, ep_cp);
                }
            );
        }
    }
    void
    request_double_response(invocation_target const& id, operation_id const& op,
            endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, id, op, ep_cp]()
                {
                    o->request_double_response(id, op, ep_cp);
                }
            );
        }
    }

    void
    connect(endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, ep_cp]()
                {
                    o->connect(ep_cp);
                }
            );
        }
    }
    void
    connection_failure(endpoint const& ep, ::std::exception_ptr ex) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, ep_cp, ex]()
                {
                    o->connection_failure(ep_cp, ex);
                }
            );
        }
    }
    void
    disconnect(endpoint const& ep) const noexcept override
    {
        shared_lock lock{mutex_};
        endpoint ep_cp{ep};
        for (auto o : observers_) {
            io_svc_->post(
                [o, ep_cp]()
                {
                    o->disconnect(ep_cp);
                }
            );
        }
    }

    void
    add_observer(connection_observer_ptr observer)
    {
        exclusive_lock lock{mutex_};
        observers_.insert(observer);
    }

    void
    remove_observer(connection_observer_ptr observer)
    {
        exclusive_lock lock{mutex_};
        observers_.erase(observer);
    }
private:
    mutex_type mutable              mutex_;
    asio_config::io_service_ptr     io_svc_;
    connection_observer_set         observers_;
};


}  /* namespace detail */
}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_OBSERVER_CONTAINER_HPP_ */
