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

#include <wire/core/connection_observer.hpp>

#include <wire/core/detail/configuration_options.hpp>
#include <wire/core/detail/observer_container.hpp>

#include <wire/encoding/buffers.hpp>

#include <wire/errors/not_found.hpp>
#include <wire/errors/user_exception.hpp>
#include <wire/errors/unexpected.hpp>

#include <wire/util/io_service_wait.hpp>
#include <wire/util/debug_log.hpp>

#include <afsm/fsm.hpp>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_priority_queue.h>

#include <iostream>
#include <atomic>
#include <map>
#include <mutex>

#if DEBUG_OUTPUT >= 3
#include <wire/core/detail/connection_fsm_observer.hpp>
#endif

namespace wire {
namespace core {
namespace detail {

struct connection_implementation;
using connection_impl_ptr   = ::std::shared_ptr< connection_implementation >;

using incoming_buffer       = ::std::array< unsigned char,
                                asio_config::incoming_buffer_size >;
using incoming_buffer_ptr   = ::std::shared_ptr< incoming_buffer >;

namespace events {

struct connect{
    endpoint                        ep;
    functional::void_callback       success;
    functional::exception_callback  fail;
};
struct connected {};
struct start{};
struct started{};   // Server connection started
struct close{};

struct connection_failure {
    ::std::exception_ptr                error;
};

struct validate_sent {};
struct receive_validate{};
struct receive_request{
    encoding::incoming_ptr              incoming;
};
struct receive_reply{
    encoding::incoming_ptr              incoming;
};
struct receive_close{};

struct receive_data{
    incoming_buffer_ptr                 buffer;
    ::std::size_t                       bytes;
};

struct send_request{
    encoding::outgoing_ptr              outgoing;
    functional::void_callback           sent;
};
struct send_reply{
    encoding::outgoing_ptr              outgoing;
};

struct write_done {};

}  // namespace events

template < typename Mutex, typename Concrete >
struct connection_fsm_def :
        ::afsm::def::state_machine<
             connection_fsm_def< Mutex, Concrete > > {
    using concrete_type = Concrete;
    using mutex_type    = Mutex;

    using this_type     = connection_fsm_def< mutex_type, concrete_type >;
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
    /** @name Guards */
    struct is_server {
        template < typename FSM, typename State >
        bool
        operator()(FSM const& fsm, State const&)
        {
            return fsm.mode_ >= server;
        }
    };
    struct is_stream_oriented {
        template < typename FSM, typename State >
        bool
        operator()(FSM const& fsm, State const&)
        {
            return fsm->is_stream_oriented();
        }
    };
    //@}
    //@{
    /** @name Actions */
    struct connect {
        template < typename FSM, typename SourceState, typename TargetState >
        void
        operator()(events::connect const& evt, FSM& fsm,
                SourceState&, TargetState&)
        {
            DEBUG_LOG_TAG(4, root_machine(fsm)->tag, "connect action");
            // Do connect async
            root_machine(fsm)->do_connect_async(evt.ep);
        }
    };
    struct on_connected {
        template < typename FSM >
        void
        operator()(events::connected const& evt, FSM& fsm,
                connecting& from, wait_validate& to)
        {
            DEBUG_LOG_TAG(3, root_machine(fsm)->tag, "on_connected action (connected)")
            ::std::swap(to.success, from.success);
            ::std::swap(to.fail, from.fail);
        }
        template < typename FSM >
        void
        operator()(events::receive_validate const& evt, FSM& fsm,
                wait_validate& from, online& to)
        {
            DEBUG_LOG_TAG(3, root_machine(fsm)->tag, "on_connected action (receive_validate)")
        }
    };
    struct server_start {
        template < typename Event, typename FSM, typename SourceState, typename TargetState >
        void
        operator()(Event const&, FSM& fsm, SourceState&, TargetState&)
        {
            root_machine(fsm)->start_server_session();
        }
    };
    struct send_validate {
        template < typename Event, typename FSM, typename SourceState, typename TargetState >
        void
        operator()(Event const&, FSM& fsm, SourceState&, TargetState&)
        {
            root_machine(fsm)->send_validate_message();
        }
    };
    struct send_close {
        template < typename FSM, typename SourceState, typename TargetState >
        void
        operator()(events::close const&, FSM& fsm, SourceState&, TargetState&)
        {
            root_machine(fsm)->send_close_message();
        }
    };
    struct send_request {
        template < typename FSM, typename SourceState, typename TargetState >
        void
        operator()(events::send_request const& req, FSM& fsm, SourceState&, TargetState&)
        {
            //root_machine(fsm)->post(&concrete_type::write_async, req.outgoing, req.sent);
            root_machine(fsm)->write_async(req.outgoing, req.sent);
        }
    };
    struct send_reply {
        template < typename FSM, typename SourceState, typename TargetState >
        void
        operator()(events::send_reply const& rep, FSM& fsm, SourceState&, TargetState&)
        {
//            root_machine(fsm)->post(&concrete_type::write_async,
//                    rep.outgoing, functional::void_callback{});
            root_machine(fsm)->write_async(rep.outgoing);
        }
    };
    struct process_incoming {
        template < typename FSM, typename SourceState, typename TargetState >
        void
        operator()(events::receive_data const& data, FSM& fsm, SourceState&, TargetState&)
        {
            root_machine(fsm)->read_incoming_message(data.buffer, data.bytes);
        }
    };
    struct dispatch_request {
        template < typename FSM, typename SourceState, typename TargetState >
        void
        operator()(events::receive_request const& req, FSM& fsm, SourceState&, TargetState&)
        {
            root_machine(fsm)->post(&concrete_type::dispatch_incoming_request, req.incoming);
        }
    };
    struct dispatch_reply {
        template < typename FSM, typename SourceState, typename TargetState >
        void
        operator()(events::receive_reply const& rep, FSM& fsm, SourceState&, TargetState&)
        {
            root_machine(fsm)->post(&concrete_type::dispatch_reply, rep.incoming);
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

        template < typename FSM >
        void
        on_enter(events::connect const& evt, FSM& fsm)
        {
            success = evt.success;
            fail    = evt.fail;
        }
        template < typename FSM >
        void
        on_exit(events::connection_failure const& err, FSM&)
        {
            if (fail) {
                try {
                    fail(err.error);
                } catch (...) {}
            }
        }

        functional::void_callback       success = nullptr;
        functional::exception_callback  fail    = nullptr;
    };

    struct wait_validate : state< wait_validate > {
        using deferred_events = type_tuple<
            events::send_request
        >;
        using internal_transitions = transition_table<
            /* Event                    |   Action          |   Guard   */
            in< events::receive_data    , process_incoming  , none              >,
            in< events::validate_sent   , none              , is_server         >,
            in< events::receive_validate, send_validate     , not_<is_server>   >
        >;

        wait_validate()
            : success{nullptr}, fail{nullptr}
        {
        }

        template < typename FSM, typename Event >
        void
        on_enter(Event const&, FSM& fsm)
        {
            DEBUG_LOG_TAG(4, root_machine(fsm)->tag, "wait validate enter")
            fsm->start_read();
        }
        template < typename FSM, typename Event >
        void
        on_exit( Event const&, FSM& fsm )
        {
            if (success) {
                success();
            } else {
                DEBUG_LOG_TAG(1, root_machine(fsm)->tag, "No success callback in wait_validate")
            }
        }
        template < typename FSM >
        void
        on_exit( events::connection_failure const& evt, FSM& fsm )
        {
            if (fail) {
                fail(evt.error);
            } else {
                DEBUG_LOG_TAG(1, root_machine(fsm)->tag, "No fail callback in wait_validate")
            }
        }

        functional::void_callback       success = nullptr;
        functional::exception_callback  fail    = nullptr;
    };

    struct online : state_machine< online > {
        struct write_idle : state<write_idle> {};
        struct write_busy : state<write_busy> {
            using deferred_events = type_tuple<
                events::send_request, events::send_reply
            >;
        };
        using initial_state = write_idle;
        using transitions = transition_table <
            /*  Start           Event                   Next            Action       */
            /* Writing to socket                                                     */
            /*----------------+-----------------------+---------------+--------------*/
            tr< write_idle    , events::send_request  , write_busy    , send_request >,
            tr< write_idle    , events::send_reply    , write_busy    , send_reply   >,
            tr< write_busy    , events::write_done    , write_idle    , none         >
        >;
        using internal_transitions = transition_table<
            /* Event                    |   Action          |   Guard   */
            /* Reading from socket and dispatching messages             */
            /*--------------------------+-------------------+-----------*/
            in< events::receive_data    ,   process_incoming,   none    >,
            in< events::receive_request ,   dispatch_request,   none    >,
            in< events::receive_reply   ,   dispatch_reply  ,   none    >,
            in< events::receive_validate,   none            ,   none    >,
            /* Miscellaneous                                            */
            /*--------------------------+-------------------+-----------*/
            in< events::connected       ,   none            ,   none    >
        >;
        template < typename FSM, typename Event >
        void
        on_enter(Event const&, FSM& fsm)
        {
            root_machine(fsm)->cancel_connect_timer();
        }
    };

    struct terminated : ::afsm::def::terminal_state< terminated > {
        template < typename Event, typename FSM >
        void
        on_enter(Event const&, FSM& fsm)
        {
            root_machine(fsm)->handle_close();
        }
    };

    using initial_state = unplugged;
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
        tr< unplugged,      events::start,              connecting,         server_start,   is_stream_oriented          >,
        tr< unplugged,      events::start,              online,             none,           not_<is_stream_oriented>    >,
        tr< connecting,     events::started,            wait_validate,      send_validate,  none                        >,
        /* Validate connection */
        tr< wait_validate,  events::receive_validate,   online,             on_connected,   is_server                   >,
        tr< wait_validate,  events::validate_sent,      online,             none,           not_<is_server>             >,
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
        connection_fsm_def<::std::mutex, connection_implementation>, ::std::mutex
#if DEBUG_OUTPUT >= 3
        , detail::conection_fsm_observer
#endif
    >;

struct connection_implementation : ::std::enable_shared_from_this<connection_implementation>,
        connection_fsm {
    using clock_type            = ::std::chrono::system_clock;
    using time_point            = ::std::chrono::time_point< clock_type >;
    using expire_duration       = ::std::chrono::milliseconds;
    using duration_type         = clock_type::duration;
    using request_number        = encoding::request::request_number;
    using atomic_counter        = ::std::atomic< request_number >;

    struct pending_reply {
        encoding::invocation_target             target;
        encoding::operation_specs::operation_id operation;
        encoding::reply_callback                reply;
        functional::exception_callback          error;
        time_point                              start;
        bool                                    sent;
    };
    struct reply_expiration {
        request_number                  number;
        time_point                      expires;

        bool
        operator < (reply_expiration const& rhs) const
        {
            return expires > rhs.expires;
        }
    };

    using pending_replies_type  = ::tbb::concurrent_hash_map<request_number, pending_reply>;
    using pending_replies_expire_queue = ::tbb::concurrent_priority_queue<reply_expiration>;

    using mutex_type            = ::std::mutex;
    using lock_guard            = ::std::lock_guard<mutex_type>;
    using optional_endpoint     = ::boost::optional<endpoint>;

    static connection_impl_ptr
    create_connection( adapter_ptr adptr, transport_type _type,
            functional::void_callback on_close );
    static connection_impl_ptr
    create_listen_connection( adapter_ptr adptr, transport_type _type,
            functional::void_callback on_close );

    connection_implementation( client_side const&, adapter_ptr adptr,
            functional::void_callback on_close)
        : adapter_{adptr},
          number_{ ++conn_counter_ },
          connector_{adptr->get_connector()},
          io_service_{adptr->io_service()},
          connection_timer_{*io_service_},
          request_no_{0},
          request_timer_{*io_service_},
          outstanding_responses_{0},
          on_close_{ on_close },
          observer_{adptr->io_service(), adptr->connection_observers()}
    {
        mode_ = client;
        DEBUG_LOG_TAG(1, tag, "Create client connection instance")
        #if DEBUG_OUTPUT >= 3
        make_observer();
        #endif
        carry_.reserve(encoding::message::max_header_size);
    }
    connection_implementation( server_side const&, adapter_ptr adptr,
            functional::void_callback on_close)
        : adapter_{adptr},
          number_{ ++conn_counter_ },
          connector_{adptr->get_connector()},
          io_service_{adptr->io_service()},
          connection_timer_{*io_service_},
          request_no_{0},
          request_timer_{*io_service_},
          outstanding_responses_{0},
          on_close_{ on_close },
          observer_{adptr->io_service(), adptr->connection_observers()}
    {
        mode_ = server;
        DEBUG_LOG_TAG(1, tag, "Create server connection instance")
        #if DEBUG_OUTPUT >= 3
        make_observer();
        #endif
        carry_.reserve(encoding::message::max_header_size);
    }

    virtual ~connection_implementation()
    {
        connection_timer_.cancel();
        request_timer_.cancel();
        DEBUG_LOG_TAG(1, tag, "Destroy connection instance")
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

    virtual bool
    is_open() const = 0;

    void
    set_connect_timer();
    void
    cancel_connect_timer();
    void
    on_connect_timeout(asio_config::error_code const& ec);

    void
    set_idle_timer();
    void
    on_idle_timeout(asio_config::error_code const& ec);
    bool
    can_drop_connection() const;

    void
    start_request_timer();
    void
    on_request_timeout(asio_config::error_code const& ec);
    void
    request_error(request_number r_no, ::std::exception_ptr ex);

    void
    connect_async(endpoint const&,
            functional::void_callback cb, functional::exception_callback eb);
    void
    handle_connected(asio_config::error_code const& ec);
    void
    send_validate_message();

    void
    add_observer(connection_observer_ptr observer)
    {
        observer_.add_observer(observer);
    }
    void
    remove_observer(connection_observer_ptr observer)
    {
        observer_.remove_observer(observer);
    }

    //@{
    /** @name Server-side session */
    /**
     * Start server session, called by listener
     */
    void
    start_session();
    /**
     * Start server session, called by FSM
     */
    void
    start_server_session();
    /**
     * Start server session callback
     * @param ec
     */
    void
    handle_started(asio_config::error_code const& ec);
    //@}

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
    process_message(encoding::message m,
            incoming_buffer::iterator& b, incoming_buffer::iterator e);
    void
    dispatch_incoming(encoding::incoming_ptr);

    void
    dispatch_reply(encoding::incoming_ptr);
    void
    dispatch_incoming_request(encoding::incoming_ptr);

    void
    send_not_found(request_number req_num, errors::not_found::subject,
            encoding::operation_specs const&);
    void
    send_exception(request_number req_num, ::std::exception_ptr ex,
            encoding::operation_specs const&);
    void
    send_exception(request_number req_num, errors::user_exception const&);
    void
    send_exception(request_number req_num, ::std::exception const&);
    void
    send_unknown_exception(request_number req_num);

    void
    invoke(encoding::invocation_target const&,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const& opts,
            encoding::outgoing&&,
            encoding::reply_callback reply,
            functional::exception_callback exception,
            functional::callback< bool > sent);
    void
    send(encoding::multiple_targets const&,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const& opts,
            encoding::outgoing&&,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    void
    forward(encoding::multiple_targets const&,
            encoding::operation_specs::operation_id const& op,
            context_type const& ctx,
            invocation_options const& opts,
            detail::dispatch_request const& req,
            encoding::reply_callback reply,
            functional::exception_callback exception,
            functional::callback< bool > sent);

    /**
     * Request sent callback
     * @param r_no
     * @param sent
     * @param one_way
     */
    void
    request_sent(request_number r_no, functional::callback< bool > sent, bool one_way);

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
        post([shared_this, method, args...]() mutable {
            (shared_this.get()->*method)(args...);
        });
    }

    void
    connection_failure(::std::exception_ptr ex);

    virtual endpoint
    local_endpoint() const = 0;
    virtual endpoint
    remote_endpoint() const = 0;

    void
    do_connect_async(endpoint const& ep)
    {
        DEBUG_LOG_TAG(3, tag, "Starting async connection operation")
        set_connect_timer();
        auto _this = shared_from_this();
        do_connect_async_impl(ep,
            [_this](asio_config::error_code const& ec)
            {
                _this->handle_connected(ec);
            });
    }

    virtual void
    do_close() = 0;

    ::std::ostream&
    tag(::std::ostream& os) const
    {
        static ::std::size_t thread_counter = 0;
        thread_local ::std::size_t thread_no = ++thread_counter;
        auto t = ::std::time(nullptr);
        os << t << " " << ::getpid() << " #" << number_ << (mode_ == server ? "s" : "c")
                << " " << thread_no << " ";
        return os;
    }
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
    do_start_session(asio_config::asio_callback cb)
    {
        throw ::std::logic_error("do_start_session is not implemented");
    }
    virtual void
    do_write_async(encoding::outgoing_ptr, asio_config::asio_rw_callback) = 0;
    virtual void
    do_read_async(incoming_buffer_ptr, asio_config::asio_rw_callback) = 0;

public:
    adapter_weak_ptr                adapter_;
    ::std::size_t const             number_;
protected:
    using carry_buffer_type = ::std::vector<unsigned char>;
    connector_weak_ptr              connector_;
    asio_config::io_service_ptr     io_service_;
    asio_config::system_timer       connection_timer_;

    atomic_counter                  request_no_;
    asio_config::system_timer       request_timer_;
    pending_replies_type            pending_replies_;
    pending_replies_expire_queue    expiration_queue_;

    encoding::incoming_ptr          incoming_;
    carry_buffer_type               carry_;
    ::std::atomic<::std::int32_t>   outstanding_responses_;
    functional::void_callback       on_close_;

    observer_container              observer_;

    static ::std::atomic< ::std::size_t >  conn_counter_;
};

::std::atomic< ::std::size_t >  connection_implementation::conn_counter_{0};

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
          transport_{ io_service_, adptr->ssl_options() }
    {
        // TODO Add verification handler for storing certificate chain
        DEBUG_LOG_TAG(3, tag, "create client-side secure connection")
    }
    template < typename T = transport_type >
    connection_impl(server_side const& s, adapter_ptr adptr,
            functional::void_callback on_close,
            detail::adapter_options const& opts = {},
            typename ::std::enable_if< is_secure< T >::value, void >::type* = nullptr)
        : connection_implementation{s, adptr, on_close},
          transport_{ io_service_, adptr->ssl_options() }
    {
        // TODO Add verification handler for storing certificate chain
        DEBUG_LOG_TAG(3, tag, "create server-side secure connection")
    }
    virtual ~connection_impl()
    {
        transport_.close();
    }

    bool
    is_stream_oriented() const override
    { return transport_traits::stream_oriented; }

    bool
    is_open() const override
    { return transport_.is_open(); }
    endpoint
    local_endpoint() const override
    {
        if (local_endpoint_.is_initialized())
            return *local_endpoint_;
        if (transport_.is_open()) {
            asio_config::error_code ec;
            auto ep = transport_.local_endpoint(ec);
            if (!ec) {
                local_endpoint_ = ep;
                return *local_endpoint_;
            }
        }

        return endpoint{};
    }
    endpoint
    remote_endpoint() const override
    {
        if (remote_endpoint_.is_initialized())
            return *remote_endpoint_;
        if (transport_.is_open()) {
            asio_config::error_code ec;
            auto ep = transport_.remote_endpoint(ec);
            if (!ec) {
                remote_endpoint_ = ep;
                return ep;
        }
        }
        return configured_endpoint_;
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
        configured_endpoint_ = ep;
        transport_.connect_async(ep, cb);
    }
    void
    do_start_session(asio_config::asio_callback cb) override
    {
        do_start_session_impl(cb);
    }

    template < typename T = transport_type >
    typename ::std::enable_if<is_secure< T >::value, void>::type
    do_start_session_impl(asio_config::asio_callback cb)
    {
        transport_.start(cb);
    }
    template < typename T = transport_type >
    typename ::std::enable_if<!is_secure< T >::value, void>::type
    do_start_session_impl(asio_config::asio_callback cb)
    {
        if (cb) cb(asio_config::error_code{});
    }
    void
    do_close() override
    {
        transport_.close();
    }
    void
    do_write_async(encoding::outgoing_ptr buffer, asio_config::asio_rw_callback cb) override
    {
        auto buff = buffer->to_buffers();
        transport_.async_write( *buff,
        [cb, buff](asio_config::error_code const& ec, ::std::size_t sz){
            cb(ec, sz);
        });
    }
    void
    do_read_async(incoming_buffer_ptr buffer, asio_config::asio_rw_callback cb) override
    {
        transport_.async_read( asio_ns::buffer(*buffer), cb );
    }
    endpoint        configured_endpoint_;
    optional_endpoint mutable   remote_endpoint_;
    optional_endpoint mutable   local_endpoint_;

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
    bool
    is_open() const override
    { return listener_.is_open(); }
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
        DEBUG_LOG_TAG(1, tag, _type << " Open endpoint " << ep)

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
