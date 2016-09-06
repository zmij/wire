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

#include <wire/core/detail/configuration_options.hpp>

#include <wire/encoding/buffers.hpp>

#include <wire/errors/not_found.hpp>
#include <wire/errors/user_exception.hpp>
#include <wire/errors/unexpected.hpp>

#include <wire/util/io_service_wait.hpp>

#include <afsm/fsm.hpp>

#include <iostream>
#include <atomic>
#include <map>
#include <mutex>

namespace wire {
namespace core {
namespace detail {

struct connection_implementation;
using connection_impl_ptr = ::std::shared_ptr< connection_implementation >;

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
    ::std::exception_ptr                error;
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
struct send_reply{
    encoding::outgoing_ptr            outgoing;
};

}  // namespace events

template < typename Mutex, typename Concrete >
struct connection_fsm_def :
        ::afsm::def::state_machine<
             connection_fsm_def< Mutex, Concrete > > {
    using concrete_type = Concrete;
    using mutex_type    = Mutex;

    using this_type     = connection_fsm_def< mutex_type, concrete_type >;
    typedef ::afsm::state_machine< this_type, mutex_type > fsm_type;
    //@{
    /** @name Typedefs for AFSM types */
    template < typename StateDef, typename ... Tags >
    using state = ::afsm::def::state<StateDef, Tags...>;
    template < typename MachineDef, typename ... Tags >
    using state_machine = ::afsm::def::state_machine<MachineDef, Tags...>;
    template < typename ... T >
    using transition_table = ::afsm::def::transition_table<T...>;
    template < typename Predicate >
    using not_ = ::psst::meta::not_<Predicate>;

    using none = ::afsm::none;

    template < typename Event, typename Action = none, typename Guard = none >
    using in = ::afsm::def::internal_transition< Event, Action, Guard>;
    template <typename SourceState, typename Event, typename TargetState,
            typename Action = none, typename Guard = none>
    using tr = ::afsm::def::transition<SourceState, Event, TargetState, Action, Guard>;
    template < typename ... T >
    using type_tuple = ::psst::meta::type_tuple<T...>;
    //@}
    //@{
    enum connection_mode {
        client,
        server,
        server_routing
    };
    //@}
    //@{
    /** @name State forwards */
    struct connecting;
    struct wait_validate;
    struct online;

    //@}
    //@{
    /** @name Actions */
    struct connect {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::connect const& evt, fsm_type& fsm, SourceState&, TargetState&)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "connect action\n";
            #endif
            // Do connect async
            fsm->do_connect_async(evt.ep);
        }
    };
    struct on_connected {
        void
        operator()(events::connected const& evt, fsm_type& fsm,
                connecting& from, wait_validate& to)
        {
            ::std::swap(to.success, from.success);
            ::std::swap(to.fail, from.fail);
        }
        void
        operator()(events::receive_validate const& evt, fsm_type& fsm,
                wait_validate& from, online& to)
        {
        }
    };
    struct on_disconnected {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::connection_failure const& evt, fsm_type& fsm,
                SourceState&, TargetState&)
        {
            #if DEBUG_OUTPUT >= 3
            ::std::cerr << "Disconnected on error\n";
            #endif
            fsm->handle_close();
        }
        template < typename SourceState, typename TargetState >
        void
        operator()(events::close const& evt, fsm_type& fsm,
                SourceState&, TargetState&)
        {
            #if DEBUG_OUTPUT >= 3
            ::std::cerr << "Disconnected gracefully\n";
            #endif
            fsm->handle_close();
        }
    };
    struct send_validate {
        template < typename Event, typename SourceState, typename TargetState >
        void
        operator()(Event const&, fsm_type& fsm, SourceState&, TargetState&)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "send validate action\n";
            #endif
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
    struct send_reply {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::send_reply const& rep, fsm_type& fsm, SourceState&, TargetState&)
        {
            fsm->write_async(rep.outgoing);
        }
    };
    struct dispatch_request {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::receive_request const& req, fsm_type& fsm, SourceState&, TargetState&)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "Dispatch request\n";
            #endif
            fsm->post(&concrete_type::dispatch_incoming_request, req.incoming);
        }
    };
    struct dispatch_reply {
        template < typename SourceState, typename TargetState >
        void
        operator()(events::receive_reply const& rep, fsm_type& fsm, SourceState&, TargetState&)
        {
            fsm->post(&concrete_type::dispatch_reply, rep.incoming);
        }
    };
    //@}

    //@{
    /** @name States */
    struct unplugged : state< unplugged > {
        using deferred_events = type_tuple<
            events::send_request
        >;
    };

    struct connecting : state< connecting > {
        using deferred_events = type_tuple<
            events::send_request
        >;

        void
        on_enter(events::connect const& evt, fsm_type& fsm)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "connecting enter\n";
            #endif
            success = evt.success;
            fail    = evt.fail;
        }
        template < typename Event >
        void
        on_exit(Event const&, fsm_type&)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "connecting exit (unexpected event)\n";
            #endif
            clear_callbacks();
        }
        void
        on_exit( events::connected const&, fsm_type& )
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "connecting exit (success)\n";
            #endif
        }
        void
        on_exit(events::connection_failure const& err, fsm_type&)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "connecting exit (fail)\n";
            #endif
            if (fail) {
                try {
                    fail(err.error);
                } catch (...) {}
            }
            clear_callbacks();
        }

        void
        clear_callbacks()
        {
            success = nullptr;
            fail    = nullptr;
        }

        functional::void_callback       success;
        functional::exception_callback  fail;
    };

    struct wait_validate : state< wait_validate > {
        using deferred_events = type_tuple<
            events::send_request
        >;

        wait_validate()
            : success{nullptr}, fail{nullptr}
        {
        }

        template < typename Event >
        void
        on_enter(Event const&, fsm_type& fsm)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "wait_validate enter\n";
            #endif
            fsm->start_read();
        }
        template < typename Event >
        void
        on_exit(Event const&, fsm_type&)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "wait_validate exit\n";
            #endif
            clear_callbacks();
        }
        void
        on_exit( events::receive_validate const&, fsm_type& )
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "wait_validate exit (success)\n";
            #endif
            if (success) {
                success();
            }
            clear_callbacks();
        }
        void
        on_exit( events::connection_failure const& evt, fsm_type& )
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "wait_validate (fail)\n";
            #endif
            if (fail) {
                fail(evt.error);
            }
            clear_callbacks();
        }

        void
        clear_callbacks()
        {
            success = nullptr;
            fail    = nullptr;
        }
        functional::void_callback       success;
        functional::exception_callback  fail;
    };

    struct online : state< online > {
        using internal_transitions = transition_table<
            in< events::send_request,       send_request,       none    >,
            in< events::send_reply,         send_reply,         none    >,
            in< events::receive_request,    dispatch_request,   none    >,
            in< events::receive_reply,      dispatch_reply,     none    >,
            in< events::receive_validate,   none,               none    >,
            in< events::connected,          none,               none    >
        >;
        template < typename Event >
        void
        on_enter(Event const&, fsm_type& fsm)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "connected enter\n";
            #endif
        }
        template < typename Event >
        void
        on_exit(Event const&, fsm_type& fsm)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "connected exit\n";
            #endif
        }
    };

    struct terminated : ::afsm::def::terminal_state< terminated > {
        template < typename Event >
        void
        on_enter(Event const&, fsm_type& fsm)
        {
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "terminated enter\n";
            #endif
            fsm->do_close();
            fsm->handle_close();
        }
    };

    using initial_state = unplugged;
    //@}
    //@{
    /** @name Guards */
    struct is_server {
        template < typename State >
        bool
        operator()(fsm_type const& fsm, State const&)
        {
            return fsm.mode_ >= server;
        }
    };
    struct is_stream_oriented {
        template < typename State >
        bool
        operator()(fsm_type const& fsm, State const&)
        {
            return fsm->is_stream_oriented();
        }
    };
    //@}

    //@{
    /** @name Transition table */
    using transitions = transition_table<
        /*  Start           Event                       Next                Action          Guard                       */
        /* Start client connection */
        tr< unplugged,      events::connect,            connecting,         connect,        none                        >,
        tr< connecting,     events::connected,          wait_validate,      on_connected,   is_stream_oriented          >,
        tr< connecting,     events::connected,          online,             none,           not_<is_stream_oriented>    >,
        /* Start server connection */
        tr< unplugged,      events::start,              wait_validate,      send_validate,  none                        >,
        tr< unplugged,      events::start,              online,             none,           not_<is_stream_oriented>    >,
        /* Validate connection */
        tr< wait_validate,  events::receive_validate,   online,             on_connected,   is_server                   >,
        tr< wait_validate,  events::receive_validate,   online,             send_validate,  not_<is_server>             >,
        /* Close connection */
        tr< unplugged,      events::close,              terminated,         none,           none                        >,
        tr< connecting,     events::close,              terminated,         none,           none                        >,
        tr< wait_validate,  events::close,              terminated,         none,           none                        >,
        tr< online,         events::close,              terminated,         send_close,     none                        >,
        tr< online,         events::receive_close,      terminated,         none,           none                        >,
        /* Connection failure */
        tr< connecting,     events::connection_failure, terminated,         none,           none                        >,
        tr< wait_validate,  events::connection_failure, terminated,         none,           none                        >,
        tr< online,         events::connection_failure, terminated,         none,           none                        >
    >;
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

using connection_fsm = ::afsm::state_machine<
        connection_fsm_def<::std::mutex, connection_implementation>, ::std::mutex >;

struct connection_implementation : ::std::enable_shared_from_this<connection_implementation>,
        connection_fsm {
    using clock_type            = ::std::chrono::system_clock;
    using time_point            = ::std::chrono::time_point< clock_type >;
    using expire_duration       = ::std::chrono::duration< ::std::int64_t, ::std::milli >;

    struct pending_reply {
        encoding::reply_callback        reply;
        functional::exception_callback  error;
        time_point                      expires;
    };
    using pending_replies_type  = ::std::unordered_map< uint32_t, pending_reply >;

    using incoming_buffer       = ::std::array< unsigned char, 1024 >;
    using incoming_buffer_ptr   = ::std::shared_ptr< incoming_buffer >;
    using mutex_type            = ::std::mutex;
    using lock_guard            = ::std::lock_guard<mutex_type>;

    static connection_impl_ptr
    create_connection( adapter_ptr adptr, transport_type _type,
            functional::void_callback on_close );
    static connection_impl_ptr
    create_listen_connection( adapter_ptr adptr, transport_type _type,
            functional::void_callback on_close );

    connection_implementation( client_side const&, adapter_ptr adptr,
            functional::void_callback on_close)
        : adapter_{adptr},
          connector_{adptr->get_connector()},
          io_service_{adptr->io_service()},
          connection_timer_{*io_service_},
          request_no_{0},
          request_timer_{*io_service_},
          outstanding_responses_{0},
          on_close_{ on_close }
    {
        #if DEBUG_OUTPUT >= 1
        ::std::cerr << "Create client connection instance\n";
        #endif
        mode_ = client;
    }
    connection_implementation( server_side const&, adapter_ptr adptr,
            functional::void_callback on_close)
        : adapter_{adptr},
          connector_{adptr->get_connector()},
          io_service_{adptr->io_service()},
          connection_timer_{*io_service_},
          request_no_{0},
          request_timer_{*io_service_},
          outstanding_responses_{0},
          on_close_{ on_close }
    {
        #if DEBUG_OUTPUT >= 1
        ::std::cerr << "Create server connection instance\n";
        #endif
        mode_ = server;
    }

    virtual ~connection_implementation()
    {
        connection_timer_.cancel();
        request_timer_.cancel();
        #if DEBUG_OUTPUT >= 1
        ::std::cerr << "Destroy connection instance\n";
        #endif
    }

    connector_ptr
    get_connector() const
    { return connector_.lock(); }

    virtual bool
    is_stream_oriented() const = 0;

    bool
    is_terminated() const
    {
        return connection_fsm::is_in_state< connection_fsm_def::terminated >();
    }

    void
    set_connection_timer();
    void
    on_connection_timeout(asio_config::error_code const& ec);
    bool
    can_drop_connection() const;

    void
    start_request_timer();
    void
    on_request_timeout(asio_config::error_code const& ec);

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
    handle_write(asio_config::error_code const& ec, ::std::size_t bytes,
            functional::void_callback cb, encoding::outgoing_ptr);

    void
    start_read();
    void
    read_async(incoming_buffer_ptr);
    void
    handle_read(asio_config::error_code const& ec, ::std::size_t bytes,
            incoming_buffer_ptr);

    void
    read_incoming_message(incoming_buffer_ptr, ::std::size_t bytes);
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
    invoke(identity const&, ::std::string const& op, context_type const& ctx,
            bool run_sync,
            encoding::outgoing&&,
            encoding::reply_callback reply,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    template < typename Handler >
    void
    post( Handler&& handler )
    {
        io_service_->post(::std::forward<Handler>(handler));
    }

    template < typename ... Args >
    void
    post( void (connection_implementation::* method)(Args ...), Args ... args)
    {
        auto shared_this = shared_from_this();
        io_service_->post([shared_this, method, args...]() mutable {
            (shared_this.get()->*method)(args...);
        });
    }

    virtual endpoint
    local_endpoint() const = 0;
    virtual endpoint
    remote_endpoint() const = 0;

    void
    do_connect_async(endpoint const& ep)
    {
        do_connect_async_impl(ep,
            ::std::bind(&connection_implementation::handle_connected, shared_from_this(),
                    ::std::placeholders::_1));
    }

    virtual void
    do_close() = 0;
private:
    virtual void
    do_connect_async_impl(endpoint const& ep, asio_config::asio_callback cb)
    {
        throw ::std::logic_error("do_connect_async is not implemented");
    }
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
    adapter_weak_ptr                adapter_;
protected:
    connector_weak_ptr              connector_;
    asio_config::io_service_ptr     io_service_;
    asio_config::system_timer       connection_timer_;

    ::std::atomic<::std::uint32_t>  request_no_;
    asio_config::system_timer       request_timer_;
    pending_replies_type            pending_replies_;
    mutex_type                      reply_mutex_;

    encoding::incoming_ptr          incoming_;
    ::std::atomic<::std::int32_t>   outstanding_responses_;
    functional::void_callback       on_close_;
};

template < typename T >
struct is_secure : ::std::false_type {};
template <>
struct is_secure< ssl_transport > : ::std::true_type {};

template < transport_type _type >
struct connection_impl : connection_implementation {
    using transport_traits    = transport_type_traits< _type >;
    using transport_type    = typename transport_traits::type;
    using socket_type        = typename transport_traits::listen_socket_type;

    template < typename T = transport_type >
    connection_impl(client_side const& c, adapter_ptr adptr,
            functional::void_callback on_close,
            typename ::std::enable_if< !is_secure< T >::value, void >::type* = nullptr)
        : connection_implementation{c, adptr, on_close}, transport_{ io_service_ }
    {
    }
    template < typename T = transport_type >
    connection_impl(server_side const& s, adapter_ptr adptr,
            functional::void_callback on_close,
            typename ::std::enable_if< !is_secure< T >::value, void >::type* = nullptr)
        : connection_implementation{s, adptr, on_close}, transport_{ io_service_ }
    {
    }
    template < typename T = transport_type >
    connection_impl(client_side const& c, adapter_ptr adptr,
            functional::void_callback on_close,
            detail::adapter_options const& opts = {},
            typename ::std::enable_if< is_secure< T >::value, void >::type* = nullptr)
        : connection_implementation{c, adptr, on_close},
          transport_{ io_service_, adptr->options().adapter_ssl }
    {
    }
    template < typename T = transport_type >
    connection_impl(server_side const& s, adapter_ptr adptr,
            functional::void_callback on_close,
            detail::adapter_options const& opts = {},
            typename ::std::enable_if< is_secure< T >::value, void >::type* = nullptr)
        : connection_implementation{s, adptr, on_close},
          transport_{ io_service_, adptr->options().adapter_ssl }
    {
    }
    virtual ~connection_impl()
    {
        transport_.close();
    }

    bool
    is_stream_oriented() const override
    { return transport_traits::stream_oriented; }

    endpoint
    local_endpoint() const override
    {
        return transport_.local_endpoint();
    }
    endpoint
    remote_endpoint() const override
    {
        return transport_.remote_endpoint();
    }

    socket_type&
    socket()
    {
        return transport_.socket();
    }
private:
    void
    do_connect_async_impl(endpoint const& ep, asio_config::asio_callback cb) override
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

    endpoint        configured_endpoint_;
    transport_type  transport_;
};

template < transport_type _type >
struct listen_connection_impl : connection_implementation {
    using session_type         = connection_impl< _type >;
    using listener_type        = transport_listener< session_type, _type >;
    using session_factory    = typename listener_type::session_factory;
    using transport_traits    = transport_type_traits< _type >;

    listen_connection_impl(adapter_ptr adptr, session_factory factory,
            functional::void_callback on_close)
        : connection_implementation{server_side{}, adptr, on_close},
          listener_(adptr->io_service(), factory)
    {
    }

    bool
    is_stream_oriented() const override
    { return transport_traits::stream_oriented; }
    endpoint
    local_endpoint() const override
    {
        util::run_until(io_service_, [&](){ return listener_.ready(); });
        return listener_.local_endpoint();
    }
    endpoint
    remote_endpoint() const override
    {
        return endpoint{};
    }
private:
    void
    do_listen(endpoint const& ep, bool reuse_port) override
    {
        #if DEBUG_OUTPUT >= 1
        ::std::cerr << "Open endpoint " << ep << "\n";
        #endif
        listener_.open(ep, reuse_port);
        auto adptr = adapter_.lock();
        if (adptr) {
            adptr->listen_connection_online(local_endpoint());
        }
    }
    void
    do_close() override
    {
        auto adptr = adapter_.lock();
        if (adptr) {
            adptr->connection_offline(local_endpoint());
        }
        listener_.close();
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
