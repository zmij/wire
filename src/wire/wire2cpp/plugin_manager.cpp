/*
 * plugin_manager.cpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#include <wire/wire2cpp/plugin_manager.hpp>
#include <wire/util/plugin.hpp>

namespace wire {
namespace idl {
namespace cpp {

struct plugin_manager::impl {
    using plugin_vector = ::std::vector< plugin_ptr >;


    plugin_vector   plugins_;

    impl(plugin_list const& list)
    {
        for (auto const& n : list) {
            add_plugin(n);
        }
    }
    ~impl()
    {
        plugins_.clear();
    }

    void
    add_plugin(::std::string const& entry)
    {
        auto delim = entry.find_last_of(':');
        if (delim == ::std::string::npos) {
            throw ::std::runtime_error{ "Invalid plugin entry name " + entry };
        }

        auto lib_name   = entry.substr(0, delim);
        auto func_name  = entry.substr(delim + 1);

        ::std::cerr << "Use generator plugin " << lib_name << "\n";
        auto& lib = util::plugin_manager::instance().get_plugin(lib_name);
        plugins_.emplace_back(lib.call< generator_plugin* >(func_name));
    }
};

plugin_manager::plugin_manager(plugin_list const& list)
    : pimpl_{ ::std::make_unique<impl> (list) }
{
}

plugin_manager::~plugin_manager() = default;

void
plugin_manager::start_compilation_unit(ast::compilation_unit const& u,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->start_compilation_unit(u, hdr, src);
    }
}

void
plugin_manager::finish_compilation_unit(ast::compilation_unit const& u,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->finish_compilation_unit(u, hdr, src);
    }
}

void
plugin_manager::generate_enum(ast::enumeration_ptr enum_,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->generate_enum(enum_, hdr, src);
    }
}

void
plugin_manager::generate_struct(ast::structure_ptr struct_,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->generate_struct(struct_, hdr, src);
    }
}

void
plugin_manager::generate_exception(ast::exception_ptr exc,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->generate_exception(exc, hdr, src);
    }
}

void
plugin_manager::generate_dispatch_interface(ast::interface_ptr iface,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->generate_dispatch_interface(iface, hdr, src);
    }
}

void
plugin_manager::generate_proxy_interface(ast::interface_ptr iface,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->generate_proxy_interface(iface, hdr, src);
    }
}

void
plugin_manager::generate_class(ast::class_ptr class_,
    source_stream& hdr, source_stream& src)
{
    for (auto p : pimpl_->plugins_) {
        p->generate_class(class_, hdr, src);
    }
}



} /* namespace cpp */
} /* namespace idl */
} /* namespace wire */
