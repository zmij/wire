/*
 * generator_plugin.hpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#ifndef WIRE_WIRE2CPP_GENERATOR_PLUGIN_HPP_
#define WIRE_WIRE2CPP_GENERATOR_PLUGIN_HPP_

#include <wire/idl/ast.hpp>
#include <wire/wire2cpp/cpp_source_stream.hpp>

namespace wire {
namespace idl {
namespace cpp {

/**
 * Base class for a generator plugin for generating additional cpp code,
 * e.g. JSON serialization
 */
class generator_plugin {
public:
    generator_plugin() {}
    virtual ~generator_plugin() {}

    /**
     * Will be called after generating includes by the cpp code
     * generator
     * @param hdr Header output stream
     * @param src Souce output stream
     */
    virtual void
    start_compilation_unit(ast::compilation_unit const& u,
        source_stream& hdr, source_stream& src) {}

    /**
     * Will be called when the main generator starts the finish compilation
     * unit method
     * @param u
     * @param hdr
     * @param src
     */
    virtual void
    finish_compilation_unit(ast::compilation_unit const& u,
        source_stream& hdr, source_stream& src) {}

    /**
     * Will be called _after_ generating the enumeration declaration
     * @param enum_
     * @param hdr Header output stream
     * @param src Souce output stream
     */
    virtual void
    generate_enum(ast::enumeration_ptr enum_,
        source_stream& hdr, source_stream& src) {}

    /**
     * Will be called _before_ closing the structure declaration to be able to
     * add member functions. Free functions can be added via at_namespace_scope
     * method of source_stream.
     * @param struct_
     * @param hdr Header output stream
     * @param src Souce output stream
     */
    virtual void
    generate_struct(ast::structure_ptr struct_,
        source_stream& hdr, source_stream& src) {}

    /**
     * Will be called _before_ closing the exception declaration to be able to
     * add member functions. Free functions can be added via at_namespace_scope
     * method of source_stream.
     * @param exc
     * @param hdr Header output stream
     * @param src Souce output stream
     */
    virtual void
    generate_exception(ast::exception_ptr exc,
        source_stream& hdr, source_stream& src) {}

    /**
     * Will be called _before_ closing the dispatch interface declaration for
     * an interface or a class to be able to add member functions. Free
     * functions can be added via at_namespace_scope method of source_stream.
     * @param iface
     * @param hdr Header output stream
     * @param src Souce output stream
     */
    virtual void
    generate_dispatch_interface(ast::interface_ptr iface,
        source_stream& hdr, source_stream& src) {}

    /**
     * Will be called _before_ closing the proxy interface declaration for
     * an interface or a class to be able to add member functions. Free
     * functions can be added via at_namespace_scope method of source_stream.
     * @param iface
     * @param hdr Header output stream
     * @param src Souce output stream
     */
    virtual void
    generate_proxy_interface(ast::interface_ptr iface,
        source_stream& hdr, source_stream& src) {}

    /**
     * Will be called _before_ closing the exception declaration to be able to
     * add member functions. Free functions can be added via at_namespace_scope
     * method of source_stream.
     * @param exc
     * @param hdr Header output stream
     * @param src Souce output stream
     */
    virtual void
    generate_class(ast::class_ptr class_,
        source_stream& hdr, source_stream& src) {}
};

using plugin_ptr    = ::std::shared_ptr< generator_plugin >;

} /* namespace cpp */
} /* namespace idl */
} /* namespace wire */

#endif /* WIRE_WIRE2CPP_GENERATOR_PLUGIN_HPP_ */
