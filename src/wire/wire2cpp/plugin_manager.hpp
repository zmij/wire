/*
 * plugin_manager.hpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#ifndef WIRE_WIRE2CPP_PLUGIN_MANAGER_HPP_
#define WIRE_WIRE2CPP_PLUGIN_MANAGER_HPP_

#include <wire/wire2cpp/generator_plugin.hpp>

namespace wire {
namespace idl {
namespace cpp {

/**
 * Class to hold a number of generator plugins and call their methods
 */
class plugin_manager : public generator_plugin {
public:
    using plugin_list   = ::std::vector< ::std::string >;
public:
    plugin_manager(plugin_list const&);
    virtual ~plugin_manager();

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
private:
    struct impl;
    using pimpl = ::std::unique_ptr<impl>;
    pimpl pimpl_;

};

} /* namespace cpp */
} /* namespace idl */
} /* namespace wire */

#endif /* WIRE_WIRE2CPP_PLUGIN_MANAGER_HPP_ */
