/*
 * connection.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#include <wire/core/connection.hpp>
#include <wire/core/adapter.hpp>
#include <wire/core/detail/connection_impl.hpp>
#include <wire/core/detail/dispatch_request.hpp>
#include <wire/core/current.hpp>
#include <wire/core/object.hpp>

#include <wire/encoding/message.hpp>
#include <wire/errors/user_exception.hpp>

#include <iterator>

namespace wire {
namespace core {

constexpr invocation_options::timeout_type invocation_options::default_timout;
const invocation_options invocation_options::unspecified{ invocation_flags::unspecified };

namespace detail {

using tcp_connection_impl           = connection_impl< transport_type::tcp >;
using ssl_connection_impl           = connection_impl< transport_type::ssl >;
using socket_connection_impl        = connection_impl< transport_type::socket >;

using tcp_listen_connection_impl    = listen_connection_impl< transport_type::tcp >;
using ssl_listen_connection_impl    = listen_connection_impl< transport_type::ssl >;
using socket_listen_connection_impl = listen_connection_impl< transport_type::socket >;

class invocation_foolproof_guard {
    functional::void_callback       destroy_;
    bool                            responded_ = false;
public:
    invocation_foolproof_guard(functional::void_callback on_destroy)
        : destroy_{on_destroy}
    {}
    ~invocation_foolproof_guard()
    {
        if (!responded_ && destroy_) {
            destroy_();
        }
    }
    void
    responded()
    {
        responded_ = true;
    }
};

connection_impl_ptr
connection_implementation::create_connection(adapter_ptr adptr, transport_type _type,
        functional::void_callback on_close)
{
    switch (_type) {
        case transport_type::tcp :
            return ::std::make_shared< tcp_connection_impl >( client_side{}, adptr, on_close );
        case transport_type::ssl:
            return ::std::make_shared< ssl_connection_impl >( client_side{}, adptr, on_close );
        case transport_type::socket :
            return ::std::make_shared< socket_connection_impl >( client_side{}, adptr, on_close );
        default:
            break;
    }
    throw errors::logic_error(_type, " connection is not implemented yet");
}

template < typename ListenConnection, typename Session >
::std::shared_ptr< ListenConnection >
create_listen_connection_impl(adapter_ptr adptr, functional::void_callback on_close)
{
    adapter_weak_ptr adptr_weak = adptr;
    return ::std::make_shared< ListenConnection >(
    adptr,
    [adptr_weak, on_close]( asio_config::io_service_ptr svc) {
        adapter_ptr a = adptr_weak.lock();
        // TODO Throw an error if adapter gone away
        if (!a) {
            throw errors::runtime_error{ "Adapter gone away" };
        }
        return ::std::make_shared< Session >( server_side{}, a, on_close );
    }, on_close);
}

connection_impl_ptr
connection_implementation::create_listen_connection(adapter_ptr adptr, transport_type _type,
        functional::void_callback on_close)
{
    adapter_weak_ptr adptr_weak = adptr;
    switch (_type) {
        case transport_type::tcp:
            return create_listen_connection_impl<
                    tcp_listen_connection_impl,
                    tcp_connection_impl >(adptr, on_close);
        case transport_type::ssl:
            return create_listen_connection_impl<
                    ssl_listen_connection_impl,
                    ssl_connection_impl >(adptr, on_close);
        case transport_type::socket:
            return create_listen_connection_impl<
                    socket_listen_connection_impl,
                    socket_connection_impl >(adptr, on_close);
        default:
            break;
    }
    throw errors::logic_error(_type, " listen connection is not implemented yet");
}

void
connection_implementation::set_connection_timer()
{
    if (can_drop_connection()) {
        // FIXME Configurable timeout
        if (connection_timer_.expires_from_now(::std::chrono::seconds{10}) > 0) {
            #if DEBUG_OUTPUT >= 5
            ::std::cerr << "Reset timer\n";
            #endif
            connection_timer_.async_wait(::std::bind(&connection_implementation::on_connection_timeout,
                    shared_from_this(), ::std::placeholders::_1));
        } else {
            #if DEBUG_OUTPUT >= 5
            ::std::cerr << "Set timer\n";
            #endif
            connection_timer_.async_wait(::std::bind(&connection_implementation::on_connection_timeout,
                    shared_from_this(), ::std::placeholders::_1));
        }
    }
}

void
connection_implementation::on_connection_timeout(asio_config::error_code const& ec)
{
    if (!ec) {
        #if DEBUG_OUTPUT >= 5
        ::std::cerr << "Timer expired\n";
        #endif
        if (can_drop_connection())
            process_event(events::close{});
    }
}

bool
connection_implementation::can_drop_connection() const
{
    return false; // Turn off connection timeout
    // TODO Check if peer endpoint has a routable bidir adapter
    //return pending_replies_.empty() && outstanding_responses_ <= 0;
}

void
connection_implementation::start_request_timer()
{
    request_timer_.expires_from_now(::std::chrono::milliseconds{500});
    request_timer_.async_wait(::std::bind(&connection_implementation::on_request_timeout,
                    shared_from_this(), ::std::placeholders::_1));
}

void
connection_implementation::request_error(request_number r_no,
        ::std::exception_ptr ex)
{
    pending_replies_type::const_accessor acc;
    if (pending_replies_.find(acc, r_no)) {
        #if DEBUG_OUTPUT >= 5
        ::std::cerr << "Request timed out\n";
        #endif
        if (acc->second.error) {
            auto err_handler = acc->second.error;
            io_service_->post(
            [err_handler, ex]()
            {
                try {
                    err_handler(ex);
                } catch(...) {}
            });
        }
        pending_replies_.erase(acc);
    }
}

void
connection_implementation::on_request_timeout(asio_config::error_code const& ec)
{
    static errors::request_timed_out err{ "Request timed out" };

    auto now = clock_type::now();
    reply_expiration r_exp;
    while (expiration_queue_.try_pop(r_exp)) {
        if (r_exp.expires <= now) {
            request_error(r_exp.number, ::std::make_exception_ptr(err));
        } else {
            expiration_queue_.push(r_exp);
            break;
        }
    }
    start_request_timer();
}

void
connection_implementation::connect_async(endpoint const& ep,
        functional::void_callback cb, functional::exception_callback eb)
{
    mode_ = client;
    process_event(events::connect{ ep, cb, eb });
}

void
connection_implementation::start_session()
{
    mode_ = server;
    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Start server session\n";
    #endif
    process_event(events::start{});
}

void
connection_implementation::listen(endpoint const& ep, bool reuse_port)
{
    mode_ = server;
    do_listen(ep, reuse_port);
}

void
connection_implementation::handle_connected(asio_config::error_code const& ec)
{
    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Handle connected\n";
    #endif
    if (!ec) {
        process_event(events::connected{});
        auto adp = adapter_.lock();
        if (adp) {
            adp->connection_online(local_endpoint(), remote_endpoint());
        }
        start_request_timer();
    } else {
        process_event(events::connection_failure{
            ::std::make_exception_ptr(errors::connection_failed(ec.message()))
        });
    }
}

void
connection_implementation::send_validate_message()
{
    encoding::outgoing_ptr out =
        ::std::make_shared<encoding::outgoing>(
                get_connector(),
                encoding::message::validate_flags);
    write_async(out);
}


void
connection_implementation::close()
{
    process_event(events::close{});
}

void
connection_implementation::send_close_message()
{
    encoding::outgoing_ptr out =
            ::std::make_shared<encoding::outgoing>(
                    get_connector(),
                    encoding::message::close);
    write_async(out);
}

void
connection_implementation::handle_close()
{
    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Handle close\n";
    #endif

    errors::connection_failed err{ "Conection closed" };
    auto ex = ::std::make_exception_ptr(err);

    connection_timer_.cancel();
    request_timer_.cancel();

    reply_expiration r_exp;
    while (expiration_queue_.try_pop(r_exp)) {
        #if DEBUG_OUTPUT >= 2
        ::std::cerr << "Notify request " << r_exp.number << "\n";
        #endif
        request_error(r_exp.number, ex);
    }

    if (on_close_)
        on_close_();
}

void
connection_implementation::write_async(encoding::outgoing_ptr out,
        functional::void_callback cb)
{
    #if DEBUG_OUTPUT >= 3
    ::std::cerr << "Send message " << out->type() << " size " << out->size() << "\n";
    #endif
    do_write_async( out,
        ::std::bind(&connection_implementation::handle_write, shared_from_this(),
                ::std::placeholders::_1, ::std::placeholders::_2, cb, out));
}

void
connection_implementation::handle_write(asio_config::error_code const& ec, ::std::size_t bytes,
        functional::void_callback cb, encoding::outgoing_ptr out)
{
    if (!ec) {
        if (cb) cb();
        set_connection_timer();
    } else {
        #if DEBUG_OUTPUT >= 2
        ::std::cerr << "Write failed " << ec.message() << "\n";
        #endif
        process_event(events::connection_failure{
            ::std::make_exception_ptr(errors::connection_failed(ec.message()))
        });
    }
}

void
connection_implementation::start_read()
{
    if (!is_terminated()) {
        incoming_buffer_ptr buffer = ::std::make_shared< incoming_buffer >();
        read_async(buffer);
    }
}

void
connection_implementation::read_async(incoming_buffer_ptr buffer)
{
    do_read_async(buffer,
        ::std::bind(&connection_implementation::handle_read, shared_from_this(),
                ::std::placeholders::_1, ::std::placeholders::_2, buffer));
}

void
connection_implementation::handle_read(asio_config::error_code const& ec, ::std::size_t bytes,
        incoming_buffer_ptr buffer)
{
    if (!ec) {
        read_incoming_message(buffer, bytes);
        start_read();
        set_connection_timer();
    } else {
        #if DEBUG_OUTPUT >= 2
        ::std::cerr << "Read failed " << ec.message() << "\n";
        #endif
        process_event(events::connection_failure{
            ::std::make_exception_ptr(errors::connection_failed(ec.message()))
        });
    }
}

void
connection_implementation::read_incoming_message(incoming_buffer_ptr buffer, ::std::size_t bytes)
{
    using encoding::message;
    auto b = buffer->begin();
    auto e = b + bytes;
    try {
        while (b != e) {
            if (incoming_) {
                incoming_->insert_back(b, e);
                if (incoming_->complete()) {
                    dispatch_incoming(incoming_);
                    incoming_.reset();
                }
            } else {
                message m;
                read(b, e, m);
                bytes -= b - buffer->begin();

                switch(m.type()) {
                    case message::validate: {
                        if (m.size > 0) {
                            throw errors::connection_failed("Invalid validate message");
                        }
                        process_event(events::receive_validate{});
                        break;
                    }
                    case message::close : {
                        if (m.size > 0) {
                            throw errors::connection_failed("Invalid close message");
                        }
                        process_event(events::receive_close{});
                        break;
                    }
                    default: {
                        if (m.size == 0) {
                            throw errors::connection_failed(
                                    "Zero sized ", m.type(), " message");
                        }
                        #if DEBUG_OUTPUT >= 3
                        ::std::cerr << "Receive message " << m.type()
                                << " size " << m.size << "\n";
                        #endif

                        encoding::incoming_ptr incoming =
                            ::std::make_shared< encoding::incoming >( get_connector(), m, b, e );
                        if (!incoming->complete()) {
                            incoming_ = incoming;
                        } else {
                            dispatch_incoming(incoming);
                        }
                    }
                }
            }
        }
    } catch (::std::exception const& e) {
        /** TODO Make it a protocol error? Can we handle it? */
        #if DEBUG_OUTPUT >= 2
        ::std::cerr << "Protocol read exception: " << e.what() << "\n";
        #endif
        process_event(events::connection_failure{
            ::std::current_exception()
        });
    }
}

void
connection_implementation::dispatch_incoming(encoding::incoming_ptr incoming)
{
    using encoding::message;
    switch (incoming->type()) {
        case message::request:
            process_event(events::receive_request{ incoming });
            break;
        case message::reply:
            process_event(events::receive_reply{ incoming });
            break;
        default:
            process_event(events::connection_failure{
                ::std::make_exception_ptr(errors::unmarshal_error{ "Unknown message type ", incoming->type() })
            });
    }
}

void
connection_implementation::invoke(encoding::invocation_target const& target, ::std::string const& op,
        context_type const& ctx,
        invocation_options const& opts,
        encoding::outgoing&& params,
        encoding::reply_callback reply,
        functional::exception_callback exception,
        functional::callback< bool > sent)
{
    using encoding::request;
    encoding::outgoing_ptr out = ::std::make_shared<encoding::outgoing>(
            get_connector(),
            encoding::message::request);
    request r{
        ++request_no_,
        encoding::operation_specs{ target, op },
        request::normal
    };
    if (ctx.empty())
        r.mode = static_cast<request::request_mode>(r.mode | request::no_context);

    #if DEBUG_OUTPUT >= 5
    ::std::cerr << "Invoke request " << target.identity << "::"
            << op << " #" << r.number << "\n";
    #endif

    write(::std::back_inserter(*out), r);
    if (!ctx.empty())
        write(::std::back_inserter(*out), ctx);
    params.close_all_encaps();
    out->insert_encapsulation(::std::move(params));
    functional::void_callback write_cb = sent ? [sent](){sent(true);} : functional::void_callback{};
    // TODO Don't insert pending reply in case of one-way invocation
    time_point expires = clock_type::now() + expire_duration{opts.timeout};
    pending_replies_.insert(::std::make_pair( r.number,
            pending_reply{ reply, exception } ));
    expiration_queue_.push(reply_expiration{ r.number, expires });
    process_event(events::send_request{ out, write_cb });

    if (opts.is_sync()) {
        auto _this = shared_from_this();
        auto r_no = r.number;
        util::run_while(io_service_, [_this, r_no](){
            pending_replies_type::const_accessor acc;
            return _this->pending_replies_.find(acc, r_no);
        });
    }
}

void
connection_implementation::dispatch_reply(encoding::incoming_ptr buffer)
{
    using namespace encoding;
    try {
        #if DEBUG_OUTPUT >= 3
        ::std::cerr << "Dispatch reply\n";
        #endif
        reply rep;
        incoming::const_iterator b = buffer->begin();
        incoming::const_iterator e = buffer->end();
        read(b, e, rep);
        pending_replies_type::const_accessor acc;
        if (pending_replies_.find(acc, rep.number)) {
            auto const& p_rep = acc->second;
            switch (rep.status) {
                case reply::success:{
                    #if DEBUG_OUTPUT >= 3
                    ::std::cerr << "Reply status is success\n";
                    #endif
                    if (p_rep.reply) {
                        incoming::encaps_guard encaps{buffer->begin_encapsulation(b)};

                        #if DEBUG_OUTPUT >= 3
                        version const& ever = encaps.encaps().encoding_version();
                        ::std::cerr << "Reply encaps v" << ever.major << "." << ever.minor
                                << " size " << encaps.size() << "\n";
                        #endif
                        try {
                            p_rep.reply(encaps->begin(), encaps->end());
                        } catch (...) {
                            // Ignore handler error
                        }
                    }
                    break;
                }
                case reply::success_no_body: {
                    #if DEBUG_OUTPUT >= 3
                    ::std::cerr << "Reply status is success without body\n";
                    #endif
                    if (p_rep.reply) {
                        try {
                            p_rep.reply( incoming::const_iterator{}, incoming::const_iterator{} );
                        } catch (...) {
                            // Ignore handler error
                        }
                    }
                    break;
                }
                case reply::no_object:
                case reply::no_facet:
                case reply::no_operation: {
                    #if DEBUG_OUTPUT >= 3
                    ::std::cerr << "Reply status is not found\n";
                    #endif
                    if (p_rep.error) {
                        incoming::encaps_guard encaps{buffer->begin_encapsulation(b)};

                        #if DEBUG_OUTPUT >= 3
                        version const& ever = encaps.encaps().encoding_version();
                        ::std::cerr << "Reply encaps v" << ever.major << "." << ever.minor
                                << " size " << encaps.size() << "\n";
                        #endif
                        encoding::operation_specs op;
                        auto b = encaps->begin();
                        read(b, encaps->end(), op);
                        encaps->read_indirection_table(b);
                        try {
                            p_rep.error(
                                ::std::make_exception_ptr(
                                    errors::not_found{
                                        static_cast< errors::not_found::subject >(
                                                rep.status - reply::no_object ),
                                        op.target.identity,
                                        op.target.facet,
                                        op.name()
                                    } )
                            );
                        } catch (...) {
                            // Ignore handler error
                        }
                    }
                    break;
                }
                case reply::user_exception:
                case reply::unknown_wire_exception:
                case reply::unknown_user_exception:
                case reply::unknown_exception: {
                    #if DEBUG_OUTPUT >= 3
                    ::std::cerr << "Reply status is an exception\n";
                    #endif
                    if (p_rep.error) {
                        incoming::encaps_guard encaps{buffer->begin_encapsulation(b)};

                        #if DEBUG_OUTPUT >= 3
                        version const& ever = encaps.encaps().encoding_version();
                        ::std::cerr << "Reply encaps v" << ever.major << "." << ever.minor
                                << " size " << encaps.size() << "\n";
                        #endif
                        errors::user_exception_ptr exc;
                        auto b = encaps->begin();
                        read(b, encaps->end(), exc);
                        encaps->read_indirection_table(b);
                        try {
                            p_rep.error(exc->make_exception_ptr());
                        } catch (...) {
                            // Ignore handler error
                        }
                    }
                    break;
                }
                default:
                    if (p_rep.error) {
                        try {
                            p_rep.error(::std::make_exception_ptr(
                                    errors::unmarshal_error{ "Unhandled reply status" } ));
                        } catch (...) {
                            // Ignore handler error
                        }
                    }
                    break;
            }
            pending_replies_.erase(acc);
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "Pending replies: " << pending_replies_.size() << "\n";
            #endif
        } else {
            // else discard the reply (it can be timed out)
            #if DEBUG_OUTPUT >= 4
            ::std::cerr << "No waiting callback for reply\n";
            #endif
        }
    } catch (::std::exception const& e) {
        #if DEBUG_OUTPUT >= 2
        ::std::cerr << "Exception when reading reply: " << e.what() << "\n";
        #endif
        process_event(events::connection_failure{ ::std::current_exception() });
    } catch (...) {
        #if DEBUG_OUTPUT >= 2
        ::std::cerr << "Exception when reading reply\n";
        #endif
        process_event(events::connection_failure{ ::std::current_exception() });
    }
}

void
connection_implementation::send_not_found(
        uint32_t req_num, errors::not_found::subject subj,
        encoding::operation_specs const& op)
{
    using namespace encoding;
    outgoing_ptr out =
            ::std::make_shared<outgoing>(get_connector(), message::reply);
    reply::reply_status status =
            static_cast<reply::reply_status>(reply::no_object + subj);
    reply rep { req_num, status };
    write(::std::back_inserter(*out), rep);
    {
        outgoing::encaps_guard guard{ out->begin_encapsulation() };
        write(::std::back_inserter(*out), op);
    }
    process_event(events::send_reply{out});
}

void
connection_implementation::send_exception(uint32_t req_num, errors::user_exception const& e)
{
    using namespace encoding;
    outgoing_ptr out =
            ::std::make_shared<outgoing>(get_connector(), message::reply);
    reply rep { req_num, reply::user_exception };
    auto o = ::std::back_inserter(*out);
    write(o, rep);
    {
        outgoing::encaps_guard guard{ out->begin_encapsulation() };
        e.__wire_write(o);
    }
    process_event(events::send_reply{out});
}

void
connection_implementation::send_exception(uint32_t req_num, ::std::exception const& e)
{
    using namespace encoding;
    outgoing_ptr out =
            ::std::make_shared<outgoing>(get_connector(), message::reply);
    reply rep { req_num, reply::unknown_user_exception };
    auto o = ::std::back_inserter(*out);
    write(o, rep);

    {
        outgoing::encaps_guard guard{ out->begin_encapsulation() };
        errors::unexpected ue { typeid(e).name(), e.what() };
        ue.__wire_write(o);
    }
    process_event(events::send_reply{out});
}

void
connection_implementation::send_unknown_exception(uint32_t req_num)
{
    using namespace encoding;
    outgoing_ptr out =
            ::std::make_shared<outgoing>(get_connector(), message::reply);
    reply rep { req_num, reply::unknown_exception };
    auto o = ::std::back_inserter(*out);
    write(o, rep);

    {
        outgoing::encaps_guard guard{ out->begin_encapsulation() };
        errors::unexpected ue {"Unknown type", "Unexpected exception not deriving from std::exception"};
        ue.__wire_write(o);
    }
    process_event(events::send_reply{out});
}

void
connection_implementation::dispatch_incoming_request(encoding::incoming_ptr buffer)
{
    using namespace encoding;
    try {
        request req;
        incoming::const_iterator b = buffer->begin();
        incoming::const_iterator e = buffer->end();
        read(b, e, req);
        //Find invocation by req.operation.identity
        adapter_ptr adp = adapter_.lock();
        if (adp) {
            // TODO Refactor upcall invocation to the adapter
            // TODO Use facet
            #if DEBUG_OUTPUT >= 3
            ::std::cerr << "Dispatch request " << req.operation.name()
                    << " to " << req.operation.identity << "\n";
            #endif

            auto disp = adp->find_object(req.operation.target.identity);
            if (disp) {
                current curr {
                    req.operation,
                    {},
                    remote_endpoint()
                };
                if (!(req.mode && request::no_context)) {
                    auto ctx_ptr = ::std::make_shared<context_type>();
                    read(b, e, *ctx_ptr);
                    curr.context = ctx_ptr;
                }

                incoming::encaps_guard encaps{ buffer->begin_encapsulation(b) };
                auto const& en = encaps.encaps();
                auto _this = shared_from_this();
                auto fpg = ::std::make_shared< invocation_foolproof_guard >(
                    [_this, req]() mutable {
                        #if DEBUG_OUTPUT >= 3
                        ::std::cerr << "Invocation to " << req.operation.identity
                                << " operation " << req.operation.operation
                                << " failed to respond";
                        #endif
                        _this->send_not_found(req.number, errors::not_found::object,
                                req.operation);
                    });
                detail::dispatch_request r{
                    buffer, en.begin(), en.end(), en.size(),
                    [_this, req, fpg](outgoing&& res) mutable {
                        outgoing_ptr out =
                                ::std::make_shared<outgoing>(_this->get_connector(), message::reply);
                        reply rep {
                            req.number,
                            res.empty() ? reply::success_no_body : reply::success
                        };
                        write(::std::back_inserter(*out), rep);
                        if (!res.empty()) {
                            res.close_all_encaps();
                            out->insert_encapsulation(::std::move(res));
                        }
                        _this->process_event(events::send_reply{out});
                        fpg->responded();
                    },
                    [_this, req, fpg](::std::exception_ptr ex) mutable {
                        try {
                            ::std::rethrow_exception(ex);
                        } catch (errors::not_found const& e) {
                            _this->send_not_found(req.number, e.subj(), req.operation);
                        } catch (errors::user_exception const& e) {
                            _this->send_exception(req.number, e);
                        } catch (::std::exception const& e) {
                            _this->send_exception(req.number, e);
                        } catch (...) {
                            _this->send_unknown_exception(req.number);
                        }
                        fpg->responded();
                    }
                };
                disp->__dispatch(r, curr);
                return;
            } else {
                #if DEBUG_OUTPUT >= 3
                ::std::cerr << "No object\n";
                #endif
            }
        } else {
            #if DEBUG_OUTPUT >= 3
            ::std::cerr << "No adapter\n";
            #endif
        }
        send_not_found(req.number, errors::not_found::object, req.operation);
    } catch (...) {
        process_event(events::connection_failure{ ::std::current_exception() });
    }
}

}  // namespace detail

connection::connection(client_side const&, adapter_ptr adp, transport_type tt,
        close_callback on_close)
    : pimpl_{}
{
    create_client_connection(adp, tt, on_close);
}

connection::connection(client_side const&, adapter_ptr adp, endpoint const& ep,
        functional::void_callback on_connect,
        functional::exception_callback on_error,
        close_callback on_close)
    : pimpl_{}
{
    create_client_connection(adp, ep.transport(), on_close);
    connect_async(ep, on_connect, on_error);
}

connection::connection(server_side const&, adapter_ptr adp, endpoint const& ep)
    : pimpl_{}
{
    pimpl_ = detail::connection_implementation::create_listen_connection(
        adp, ep.transport(),
        [](){
            #if DEBUG_OUTPUT >= 1
            ::std::cerr << "Server connection on close\n";
            #endif
        });
    pimpl_->listen(ep);
}

connection::~connection()
{
    #if DEBUG_OUTPUT >= 1
    ::std::cerr << "Destroy connection façade\n";
    #endif
}

void
connection::create_client_connection(adapter_ptr adp, transport_type tt,
        close_callback on_close)
{
    pimpl_ = detail::connection_implementation::create_connection(
            adp, tt,
            [this, on_close](){
                #if DEBUG_OUTPUT >= 1
                ::std::cerr << "Client connection on close\n";
                #endif
                if (on_close)
                    on_close(this);
            });
}

connector_ptr
connection::get_connector() const
{
    assert(pimpl_.get() && "Connection implementation is not set");
    return pimpl_->get_connector();
}

void
connection::connect_async(endpoint const& ep,
        functional::void_callback       on_connect,
        functional::exception_callback  on_error)
{
    assert(pimpl_.get() && "Connection implementation is not set");
    pimpl_->connect_async(ep, on_connect, on_error);
}

void
connection::set_adapter(adapter_ptr adp)
{
    assert(pimpl_.get() && "Connection implementation is not set");
    pimpl_->adapter_= adp;
}

void
connection::close()
{
    assert(pimpl_.get() && "Connection implementation is not set");
    pimpl_->close();
}

void
connection::invoke(encoding::invocation_target const& target, ::std::string const& op,
        context_type const& ctx,
        invocation_options const& opts,
        encoding::outgoing&& params,
        encoding::reply_callback reply,
        functional::exception_callback exception,
        functional::callback< bool > sent)
{
    assert(pimpl_.get() && "Connection implementation is not set");
    pimpl_->invoke(target, op, ctx, opts, ::std::move(params), reply, exception, sent);
}

endpoint
connection::local_endpoint() const
{
    assert(pimpl_.get() && "Connection implementation is not set");
    return pimpl_->local_endpoint();
}

endpoint
connection::remote_endpoint() const
{
    assert(pimpl_.get() && "Connection implementation is not set");
    return pimpl_->remote_endpoint();
}

}  // namespace core
}  // namespace wire
