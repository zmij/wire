/*
 * connection_impl.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_
#define WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_

#include <wire/core/transport.hpp>
#include <wire/encoding/buffers.hpp>

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
typedef std::shared_ptr< connection_impl_base > connection_impl_ptr;

namespace events {

struct connect{
	endpoint						ep;
	callbacks::void_callback		success;
	callbacks::exception_callback	fail;
};
struct connected {};
struct start{};
struct close{};

struct connection_failure {
	std::exception_ptr				error;
};

struct receive_validate{};
struct receive_request{
	encoding::incoming_ptr			incoming;
};
struct receive_reply{
	encoding::incoming_ptr			incoming;
};
struct receive_close{};

struct send_request{
	encoding::outgoing_ptr			outgoing;
	callbacks::void_callback		sent;
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
	typedef boost::msm::front::none none;
	template < typename T >
	using Not = boost::msm::front::euml::Not_< T >;
	//@}
	//@{
	typedef ::boost::msm::back::state_machine< connection_fsm_ > fsm_type;
	typedef Concrete concrete_type;
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
		operator()(events::receive_request const&, fsm_type& fsm, SourceState&, TargetState&)
		{
			std::cerr << "Dispatch request\n";
		}
	};
	struct dispatch_reply {
		template < typename SourceState, typename TargetState >
		void
		operator()(events::receive_reply const& rep, fsm_type& fsm, SourceState&, TargetState&)
		{
			std::cerr << "Dispatch reply\n";
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
			fail	= evt.fail;
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

		callbacks::void_callback		success;
		callbacks::exception_callback	fail;
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
		callbacks::void_callback		success;
		callbacks::exception_callback	fail;
	};

	struct connected : ::boost::msm::front::state<> {
		struct internal_transition_table : ::boost::mpl::vector<
			/*			Event 						Action				Guard	*/
			Internal<	events::send_request,		send_request,		none	>,
			Internal<	events::receive_request,	dispatch_request,	none	>,
			Internal<	events::receive_reply,		dispatch_reply,		none	>,
			Internal<	events::receive_validate,	none,				none	>
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

	typedef unplugged initial_state;
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
			/*	Start			Event						Next				Action			Guard			*/
		/* Start client connection */
		Row<	unplugged,		events::connect,			connecting,			connect,		none					>,
		Row<	connecting,		events::connected,			wait_validate,		on_connected,	is_stream_oriented		>,
		Row<	connecting,		events::connected,			connected,			none,			Not<is_stream_oriented>	>,
		/* Start server connection */
		Row<	unplugged,		events::start,				wait_validate,		send_validate,	none					>,
		Row<	unplugged,		events::start,				connected,			none,			Not<is_stream_oriented>	>,
		/* Validate connection */
		Row<	wait_validate,	events::receive_validate,	connected,			none,			is_server				>,
		Row<	wait_validate,	events::receive_validate,	connected,			send_validate,	Not<is_server>			>,
		/* Close connection */
		Row<	unplugged,		events::close,				terminated,			none,			none					>,
		Row<	connecting,		events::close,				terminated,			none,			none					>,
		Row<	wait_validate,	events::close,				terminated,			none,			none					>,
		Row<	connected,		events::close,				terminated,			send_close,		none					>,
		Row<	connected,		events::receive_close,		terminated,			none,			none					>,
		/* Connection failure */
		Row<	connecting,		events::connection_failure,	terminated,			none,			none					>,
		Row<	wait_validate,	events::connection_failure,	terminated,			none,			none					>,
		Row<	connected,		events::connection_failure,	terminated,			none,			none					>
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
		encoding::reply_callback		reply;
		callbacks::exception_callback	error;
	};
	typedef std::map< uint32_t, pending_reply >	pending_replies_type;

	typedef std::array< unsigned char, 1024 > incoming_buffer;
	typedef std::shared_ptr< incoming_buffer > incoming_buffer_ptr;

	static connection_impl_ptr
	create_connection( asio_config::io_service_ptr io_svc, transport_type _type );

	connection_impl_base() : request_no_{0} {}
	virtual ~connection_impl_base() {}

	virtual bool
	is_stream_oriented() const = 0;

	void
	connect_async(endpoint const&,
			callbacks::void_callback cb, callbacks::exception_callback eb);
	virtual void
	do_connect_async(endpoint const& ep, asio_config::asio_callback cb) = 0;
	void
	handle_connected(asio_config::error_code const& ec);
	void
	send_validate_message();

	void
	close();
	virtual void
	do_close() = 0;
	void
	send_close_message();

	void
	write_async(encoding::outgoing_ptr, callbacks::void_callback cb = nullptr);
	virtual void
	do_write_async(encoding::outgoing_ptr, asio_config::asio_rw_callback) = 0;
	void
	handle_write(asio_config::error_code const& ec, std::size_t bytes,
			callbacks::void_callback cb, encoding::outgoing_ptr);

	void
	start_read();
	void
	read_async(incoming_buffer_ptr);
	virtual void
	do_read_async(incoming_buffer_ptr, asio_config::asio_rw_callback) = 0;
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
	invoke_async(identity const&, std::string const& op,
			encoding::outgoing&&,
			encoding::reply_callback reply,
			callbacks::exception_callback exception,
			callbacks::callback< bool > sent);

	std::atomic<uint32_t>	request_no_;
	encoding::incoming_ptr	incoming_;
	pending_replies_type	pending_replies_;
};

template < transport_type _type >
struct connection_impl : connection_impl_base {
	typedef transport_type_traits< _type >	transport_traits;
	typedef typename transport_traits::type	transport_type;

	connection_impl(asio_config::io_service_ptr io_svc)
		: transport_(io_svc)
	{
	}
	virtual ~connection_impl() {}

	bool
	is_stream_oriented() const override
	{ return transport_traits::stream_oriented; }

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

	endpoint		endpoint_;
	transport_type	transport_;
};

}  // namespace detail
}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_DETAIL_CONNECTION_IMPL_HPP_ */
