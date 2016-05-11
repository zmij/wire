/*
 * connection_impl.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_
#define WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_

#include <wire/core/transport.hpp>
#include <wire/core/adapter.hpp>
#include <wire/core/connector.hpp>

#include <wire/encoding/buffers.hpp>

#include <wire/errors/user_exception.hpp>
#include <wire/errors/unexpected.hpp>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/operator.hpp>

#include <iostream>
#include <atomic>
#include <map>

namespace wire {
namespace core {
namespace detail {

struct connection_impl_base;
using connection_impl_ptr = std::shared_ptr< connection_impl_base >;

namespace events {

struct connect{
    endpoint                        ep;
    functional::void_callback       success;
    functional::exception_callback  fail;
};
struct connected {};
struct start{};
struct close{};

struct connection_failure {
    std::exception_ptr                error;
};

struct receive_validate{};
struct receive_request{
    encoding::incoming_ptr            incoming;
};
struct receive_reply{
    encoding::incoming_ptr            incoming;
};
struct receive_close{};

struct send_request{
    encoding::outgoing_ptr            outgoing;
    functional::void_callback         sent;
};
struct send_reply{};

}  // namespace events

template < typename Concrete >
struct connection_fsm_ : ::boost::msm::front::state_machine_def< connection_fsm_< Concrete > > {
    //@{
    /** @name Typedefs for MSM types */
    template < typename ... T >
    using Row = boost::msm::front::Row< T ... >;
    template < typename ... T >
    using Internal = boost::msm::front::Internal< T ... >;
    using none = boost::msm::front::none;
    template < typename T >
    using Not = boost::msm::front::euml::Not_< T >;
    //@}
    //@{
    typedef ::boost::msm::back::state_machine< connection_fsm_ > fsm_type;
    using concrete_type = Concrete;
    enum connection_mode {
        client,
        server
    };
    //@}
    //@{
    /** @name State forwards */
    struct connecting;
    struct wait_validate;

    //@}
    //@{
    /** @name Actions */
    struct connect {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::connect const& evt, fsm_type& fsm, SourceState&, TargetState&)
        {
            std::cerr << "connect action\n";
            // Do connect async
            fsm->do_connect_async(evt.ep,
                std::bind( &concrete_type::handle_connected,
                    fsm->shared_from_this(), std::placeholders::_1));
        }
    };
    struct on_connected {
        void
        operator()(events::connected const& evt, fsm_type& fsm,
                connecting& from, wait_validate& to)
        {
            to.success = from.success;
            to.fail = from.fail;
        }
    };
    struct on_disconnected {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::connection_failure const& evt, fsm_type& fsm,
                SourceState&, TargetState&)
        {
            ::std::cerr << "Disconnected on error\n";
            fsm->handle_close();
        }
        template < typename SourceState, typename TargetState >
        void
        operator()(events::close const& evt, fsm_type& fsm,
                SourceState&, TargetState&)
        {
            ::std::cerr << "Disconnected gracefully\n";
            fsm->handle_close();
        }
    };
    struct send_validate {
        template < typename Event, typename SourceState, typename TargetState >
        void
        operator()(Event const&, fsm_type& fsm, SourceState&, TargetState&)
        {
            std::cerr << "send validate action\n";
            fsm->send_validate_message();
        }
    };
    struct send_close {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::close const&, fsm_type& fsm, SourceState&, TargetState&)
        {
            fsm->send_close_message();
        }
    };
    struct send_request {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::send_request const& req, fsm_type& fsm, SourceState&, TargetState&)
        {
            fsm->write_async(req.outgoing, req.sent);
        }
    };
    struct dispatch_request {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::receive_request const& req, fsm_type& fsm, SourceState&, TargetState&)
        {
            std::cerr << "Dispatch request\n";
            fsm->dispatch_incoming_request(req.incoming);
        }
    };
    struct dispatch_reply {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::receive_reply const& rep, fsm_type& fsm, SourceState&, TargetState&)
        {
            fsm->dispatch_reply(rep.incoming);
        }
    };
    //@}

    //@{
    /** @name States */
    struct unplugged : ::boost::msm::front::state<> {
        typedef ::boost::mpl::vector<
            events::send_request
        > deferred_events;
    };

    struct connecting : ::boost::msm::front::state<> {
        typedef ::boost::mpl::vector<
            events::send_request
        > deferred_events;

        void
        on_entry(events::connect const& evt, fsm_type& fsm)
        {
            std::cerr << "connecting enter\n";
            success = evt.success;
            fail    = evt.fail;
        }
        template < typename Event >
        void
        on_exit(Event const&, fsm_type&)
        {
        }
        void
        on_exit( events::connected const&, fsm_type& )
        {
            std::cerr << "connecting exit (success)\n";
        }
        void
        on_exit(events::connection_failure const& err, fsm_type&)
        {
            std::cerr << "connecting exit (fail)\n";
            if (fail) {
                fail(err.error);
            }
        }

        functional::void_callback       success;
        functional::exception_callback  fail;
    };

    struct wait_validate : ::boost::msm::front::state<> {
        typedef ::boost::mpl::vector<
            events::send_request
        > deferred_events;

        template < typename Event >
        void
        on_entry(Event const&, fsm_type& fsm)
        {
            std::cerr << "wait_validate enter\n";
            fsm->start_read();
        }
        template < typename Event >
        void
        on_exit(Event const&, fsm_type&)
        {
            std::cerr << "wait_validate exit\n";
        }
        void
        on_exit( events::receive_validate const&, fsm_type& )
        {
            std::cerr << "wait_validate (success)\n";
            if (success) {
                success();
            }
        }
        void
        on_exit( events::connection_failure const& evt, fsm_type& )
        {
            std::cerr << "wait_validate (fail)\n";
            if (fail) {
                fail(evt.error);
            }
        }
        functional::void_callback       success;
        functional::exception_callback  fail;
    };

    struct connected : ::boost::msm::front::state<> {
        struct internal_transition_table : ::boost::mpl::vector<
            /*           Event                       Action               Guard    */
            Internal<    events::send_request,       send_request,        none    >,
            Internal<    events::receive_request,    dispatch_request,    none    >,
            Internal<    events::receive_reply,      dispatch_reply,      none    >,
            Internal<    events::receive_validate,   none,                none    >
        > {};
        template < typename Event >
        void
        on_entry(Event const&, fsm_type& fsm)
        {
            std::cerr << "connected enter\n";
        }
        template < typename Event >
        void
        on_exit(Event const&, fsm_type& fsm)
        {
            std::cerr << "connected exit\n";
        }
    };

    struct terminated : ::boost::msm::front::terminate_state<> {
        template < typename Event >
        void
        on_entry(Event const&, fsm_type& fsm)
        {
            std::cerr << "terminated enter\n";
            fsm->do_close();
        }
    };

    using initial_state = unplugged;
    //@}
    //@{
    /** @name Guards */
    struct is_server {
        template < typename Event, typename SourceState, typename TargetState >
        bool
        operator()(Event const&, fsm_type& fsm, SourceState&, TargetState&)
        {
            return fsm.mode_ == server;
        }
    };
    struct is_stream_oriented {
        template < typename Event, typename SourceState, typename TargetState >
        bool
        operator()(Event const&, fsm_type& fsm, SourceState&, TargetState&)
        {
            return fsm->is_stream_oriented();
        }
    };
    //@}

    //@{
    /** @name Transition table */
    struct transition_table : ::boost::mpl::vector<
            /*    Start            Event                        Next                Action            Guard            */
        /* Start client connection */
        Row<    unplugged,        events::connect,              connecting,           connect,         none                    >,
        Row<    connecting,       events::connected,            wait_validate,        on_connected,    is_stream_oriented      >,
        Row<    connecting,       events::connected,            connected,            none,            Not<is_stream_oriented> >,
        /* Start server connection */
        Row<    unplugged,        events::start,                wait_validate,        send_validate,    none                   >,
        Row<    unplugged,        events::start,                connected,            none,            Not<is_stream_oriented> >,
        /* Validate connection */
        Row<    wait_validate,    events::receive_validate,     connected,            none,            is_server               >,
        Row<    wait_validate,    events::receive_validate,     connected,            send_validate,   Not<is_server>          >,
        /* Close connection */
        Row<    unplugged,        events::close,                terminated,           none,            none                    >,
        Row<    connecting,       events::close,                terminated,           none,            none                    >,
        Row<    wait_validate,    events::close,                terminated,           none,            none                    >,
        Row<    connected,        events::close,                terminated,           send_close,      none                    >,
        Row<    connected,        events::receive_close,        terminated,           none,            none                    >,
        /* Connection failure */
        Row<    connecting,       events::connection_failure,   terminated,           on_disconnected, none                    >,
        Row<    wait_validate,    events::connection_failure,   terminated,           on_disconnected, none                    >,
        Row<    connected,        events::connection_failure,   terminated,           on_disconnected, none                    >
    > {};
    //@}

    concrete_type*
    operator ->()
    {
        return static_cast<concrete_type*>(this);
    }
    concrete_type const*
    operator ->() const
    {
        return static_cast<concrete_type const*>(this);
    }
    connection_mode mode_;
};

typedef ::boost::msm::back::state_machine< connection_fsm_< connection_impl_base > > connection_fsm;

struct connection_impl_base : ::std::enable_shared_from_this<connection_impl_base>,
        connection_fsm {
    struct pending_reply {
        encoding::reply_callback        reply;
        functional::exception_callback  error;
    };
    using pending_replies_type    = std::map< uint32_t, pending_reply >;

    using incoming_buffer        = std::array< unsigned char, 1024 >;
    using incoming_buffer_ptr    = std::shared_ptr< incoming_buffer >;

    static connection_impl_ptr
    create_connection( connector_ptr cnctr, asio_config::io_service_ptr io_svc, transport_type _type );
    static connection_impl_ptr
    create_listen_connection( adapter_ptr adptr, transport_type _type );

    connection_impl_base(connector_ptr cnctr, asio_config::io_service_ptr io_svc)
        : connector_{cnctr}, io_service_{io_svc}, request_no_{0}
    {
        connection_fsm::start();
    }
    connection_impl_base(adapter_ptr adptr)
        : adapter_{adptr},
          connector_{adptr->get_connector()},
          io_service_{adptr->get_connector()->io_service()},
          request_no_{0}
    {
    }
    virtual ~connection_impl_base()
    {
        ::std::cerr << "Destroy connection instance\n";
    }

    connector_ptr
    get_connector() const
    { return connector_.lock(); }

    virtual bool
    is_stream_oriented() const = 0;

    void
    connect_async(endpoint const&,
            functional::void_callback cb, functional::exception_callback eb);
    void
    handle_connected(asio_config::error_code const& ec);
    void
    send_validate_message();

    void
    start_session();

    void
    listen(endpoint const&, bool reuse_port = false);

    void
    close();
    void
    send_close_message();
    void
    handle_close();

    void
    write_async(encoding::outgoing_ptr, functional::void_callback cb = nullptr);
    void
    handle_write(asio_config::error_code const& ec, std::size_t bytes,
            functional::void_callback cb, encoding::outgoing_ptr);

    void
    start_read();
    void
    read_async(incoming_buffer_ptr);
    void
    handle_read(asio_config::error_code const& ec, std::size_t bytes,
            incoming_buffer_ptr);

    void
    read_incoming_message(incoming_buffer_ptr, std::size_t bytes);
    void
    dispatch_incoming(encoding::incoming_ptr);

    void
    dispatch_reply(encoding::incoming_ptr);
    void
    dispatch_incoming_request(encoding::incoming_ptr);

    void
    send_not_found(uint32_t req_num, errors::not_found::subject,
            encoding::operation_specs const&);
    void
    send_exception(uint32_t req_num, errors::user_exception const&);
    void
    send_exception(uint32_t req_num, ::std::exception const&);
    void
    send_unknown_exception(uint32_t req_num);

    void
    invoke(identity const&, std::string const& op, context_type const& ctx,
            bool run_sync,
            encoding::outgoing&&,
            encoding::reply_callback reply,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    template < typename Pred >
    void
    wait_for( Pred pred ) const
    {
        while(!pred()) {
            io_service_->poll();
        }
    }
    template < typename Pred >
    void
    wait_until( Pred pred ) const
    {
        while(pred()) {
            io_service_->poll();
        }
    }
    virtual endpoint
    local_endpoint() const = 0;

    virtual void
    do_connect_async(endpoint const& ep, asio_config::asio_callback cb)
    {
        throw ::std::logic_error("do_connect_async is not implemented");
    }
    virtual void
    do_close() = 0;
private:
    virtual void
    do_listen(endpoint const&, bool reuse_port)
    {
        throw ::std::logic_error("do_listen is not implemented");
    }
    virtual void
    do_write_async(encoding::outgoing_ptr, asio_config::asio_rw_callback) = 0;
    virtual void
    do_read_async(incoming_buffer_ptr, asio_config::asio_rw_callback) = 0;
public:
    adapter_weak_ptr            adapter_;
protected:
    connector_weak_ptr          connector_;
    asio_config::io_service_ptr io_service_;
    ::std::atomic<uint32_t>     request_no_;
    encoding::incoming_ptr      incoming_;
    pending_replies_type        pending_replies_;
};

template < transport_type _type >
struct connection_impl : connection_impl_base {
    using transport_traits    = transport_type_traits< _type >;
    using transport_type    = typename transport_traits::type;
    using socket_type        = typename transport_traits::listen_socket_type;

    connection_impl(connector_ptr cnctr, asio_config::io_service_ptr io_svc)
        : connection_impl_base{cnctr, io_svc}, transport_{io_svc}
    {
    }
    connection_impl(adapter_ptr adptr)
        : connection_impl_base{adptr}, transport_{ io_service_ }
    {
    }
    virtual ~connection_impl() {}

    bool
    is_stream_oriented() const override
    { return transport_traits::stream_oriented; }

    endpoint
    local_endpoint() const override
    {
        return transport_.local_endpoint();
    }

    socket_type&
    socket()
    {
        return transport_.socket();
    }
private:
    void
    do_connect_async(endpoint const& ep, asio_config::asio_callback cb) override
    {
        transport_.connect_async(ep, cb);
    }
    void
    do_close() override
    {
        transport_.close();
    }
    void
    do_write_async(encoding::outgoing_ptr buffer, asio_config::asio_rw_callback cb) override
    {
        transport_.async_write( buffer->to_buffers(), cb );
    }
    void
    do_read_async(incoming_buffer_ptr buffer, asio_config::asio_rw_callback cb) override
    {
        transport_.async_read( ASIO_NS::buffer(*buffer), cb );
    }

    endpoint        endpoint_;
    transport_type    transport_;
};

template < transport_type _type >
struct listen_connection_impl : connection_impl_base {
    using session_type         = connection_impl< _type >;
    using listener_type        = transport_listener< session_type, _type >;
    using session_factory    = typename listener_type::session_factory;
    using transport_traits    = transport_type_traits< _type >;

    listen_connection_impl(adapter_ptr adptr, session_factory factory)
        : connection_impl_base{adptr}, listener_(io_service_, factory)
    {
    }

    bool
    is_stream_oriented() const override
    { return transport_traits::stream_oriented; }
    endpoint
    local_endpoint() const override
    {
        wait_for([&](){ return listener_.ready(); });
        return listener_.local_endpoint();
    }
private:
    void
    do_listen(endpoint const& ep, bool reuse_port) override
    {
        listener_.open(ep, reuse_port);
    }
    void
    do_close() override
    {
    }
    void
    do_write_async(encoding::outgoing_ptr buffer, asio_config::asio_rw_callback cb) override
    {
    }
    void
    do_read_async(incoming_buffer_ptr buffer, asio_config::asio_rw_callback cb) override
    {
    }

    listener_type    listener_;
};

}  // namespace detail
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_ */
