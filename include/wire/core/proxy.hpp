/*
 * proxy.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_PROXY_HPP_
#define WIRE_CORE_PROXY_HPP_

#include <wire/core/proxy_fwd.hpp>
#include <wire/core/callbacks.hpp>
#include <wire/core/context.hpp>

#include <future>
#include <functional>
#include <exception>

namespace wire {
namespace core {

class object_proxy : public ::std::enable_shared_from_this< object_proxy > {
public:

public:

	void
	wire_ping(context_type const& = no_context);

	void
	wire_ping_async(
			callbacks::void_callback		response,
			callbacks::exception_callback	exception = nullptr,
			callbacks::callback< bool >		sent = nullptr,
			context_type const& = no_context
	);

	template< template< typename > class _Promise = std::promise >
	auto
	wire_ping_async(context_type const& ctx = no_context)
		-> decltype(std::declval<_Promise<void>().get_future())
	{
		auto promise = ::std::make_shared< _Promise<void> >();

		wire_ping_async(
			[promise]()
			{
				promise->set_value();
			},
			[promise](::std::exception_ptr ex)
			{
				promise->set_exception(::std::move(ex));
			},
			nullptr, ctx
		);

		return promise->get_future();
	}
};

}  // namespace core
}  // namespace wire

#endif /* WIRE_CORE_PROXY_HPP_ */
