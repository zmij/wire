/*
 * proxy.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <wire/core/proxy.hpp>

namespace wire {
namespace core {

bool
object_proxy::wire_is_a(::std::string const& type_id, context_type const& ctx)
{
	auto future = wire_is_a_async(type_id, ctx);
	return future.get();
}

void
object_proxy::wire_is_a_async(::std::string const& type_id,
			callbacks::callback< bool >		response,
			callbacks::exception_callback	exception,
			callbacks::callback< bool >		sent,
			context_type const&				ctx)
{

}

void
object_proxy::wire_ping(context_type const& ctx)
{
	auto future = wire_ping_async(ctx);
	future.get();
}

void
object_proxy::wire_ping_async(
		callbacks::void_callback		response,
		callbacks::exception_callback	exception,
		callbacks::callback< bool >		sent,
		context_type const& 			ctx
)
{

}

::std::string
object_proxy::wire_type(context_type const& ctx)
{
	auto future = wire_type_async(ctx);
	return std::move(future.get());
}

void
object_proxy::wire_type_async(
		callbacks::callback< std::string const& >	response,
		callbacks::exception_callback				exception,
		callbacks::callback< bool >					sent,
		context_type const&							ctx
)
{

}

::std::vector< ::std::string >
object_proxy::wire_types(context_type const& ctx)
{
	auto future = wire_types_async(ctx);
	return future.get();
}

void
object_proxy::wire_types_async(
		callbacks::callback< ::std::vector< ::std::string > const& >	result,
		callbacks::exception_callback					exception,
		callbacks::callback<bool>						sent,
		context_type const&								ctx
)
{

}

}  // namespace core
}  // namespace wire
