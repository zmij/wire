/*
 * dummy_generator_plugin.cpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#include "dummy_generator_plugin.hpp"

namespace wire {
namespace idl {
namespace test {

using cpp::off;
using cpp::mod;

dummy_generator_plugin::dummy_generator_plugin()
{
}

dummy_generator_plugin::~dummy_generator_plugin()
{
}

void
dummy_generator_plugin::start_compilation_unit(ast::compilation_unit const& u,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin start compilation unit\n";
}

void
dummy_generator_plugin::finish_compilation_unit(ast::compilation_unit const& u,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin finish compilation unit\n";
}

void
dummy_generator_plugin::generate_enum(ast::enumeration_ptr enum_,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin generate enum\n";
}

void
dummy_generator_plugin::generate_struct(ast::structure_ptr struct_,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin generate struct\n";
}

void
dummy_generator_plugin::generate_exception(ast::exception_ptr exc,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin generate exception\n";
}

void
dummy_generator_plugin::generate_dispatch_interface(ast::interface_ptr iface,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin generate dispatch interface\n";
}

void
dummy_generator_plugin::generate_proxy_interface(ast::interface_ptr iface,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin generate proxy interface\n";
}

void
dummy_generator_plugin::generate_class(ast::class_ptr class_,
    source_stream& hdr, source_stream& src)
{
    hdr << off << "// dummy plugin generate class\n";
}



} /* namespace test */
} /* namespace idl */
} /* namespace wire */

::wire::idl::test::dummy_generator_plugin*
make_plugin()
{
    return new ::wire::idl::test::dummy_generator_plugin();
}
