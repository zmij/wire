/*
 * dummy_generator_plugin.hpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#ifndef WIRE_TEST_CONNECTOR_DUMMY_GENERATOR_PLUGIN_HPP_
#define WIRE_TEST_CONNECTOR_DUMMY_GENERATOR_PLUGIN_HPP_

#include <wire/wire2cpp/generator_plugin.hpp>

namespace wire {
namespace idl {
namespace test {

class dummy_generator_plugin: public cpp::generator_plugin {
public:
    using source_stream = cpp::source_stream;
public:
    dummy_generator_plugin();
    virtual ~dummy_generator_plugin();

    void
    start_compilation_unit(ast::compilation_unit const& u,
        source_stream& hdr, source_stream& src) override;
    void
    finish_compilation_unit(ast::compilation_unit const& u,
        source_stream& hdr, source_stream& src) override;
    void
    generate_enum(ast::enumeration_ptr enum_,
        source_stream& hdr, source_stream& src) override;
    void
    generate_struct(ast::structure_ptr struct_,
        source_stream& hdr, source_stream& src) override;
    void
    generate_exception(ast::exception_ptr exc,
        source_stream& hdr, source_stream& src) override;
    void
    generate_dispatch_interface(ast::interface_ptr iface,
        source_stream& hdr, source_stream& src) override;
    void
    generate_proxy_interface(ast::interface_ptr iface,
        source_stream& hdr, source_stream& src) override;
    void
    generate_class(ast::class_ptr class_,
        source_stream& hdr, source_stream& src) override;
};

} /* namespace test */
} /* namespace idl */
} /* namespace wire */

extern "C" {

::wire::idl::test::dummy_generator_plugin*
make_plugin();

}

#endif /* WIRE_TEST_CONNECTOR_DUMMY_GENERATOR_PLUGIN_HPP_ */
