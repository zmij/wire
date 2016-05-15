/*
 * fwd_generator.cpp
 *
 *  Created on: 16 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/wire2cpp/fwd_generator.hpp>

namespace wire {
namespace idl {
namespace cpp {

namespace fs = boost::filesystem;

fwd_generator::fwd_generator(generate_options const& opts,
        preprocess_options const& ppo,
        ast::global_namespace_ptr ns)
    : ns_{ns}, unit_{ns->current_compilation_unit()},
      header_{ fs::path{unit_->name}.parent_path(),
            fs::path{ opts.header_include_dir }, ppo.include_dirs }
{
    auto cwd = fs::current_path();

    fs::path origin(unit_->name);
    fs::path header_path{ opts.header_output_dir };

    if (header_path.empty()) {
        header_path = fs::current_path();
    }
    if (header_path.is_relative()) {
        header_path = fs::absolute(header_path);
    }
    if (!fs::exists(header_path)) {
        fs::create_directories(header_path);
    }
    header_path = fs::canonical(header_path);

    header_path /= origin.filename().stem().string() + "_fwd.hpp";
    header_.open(header_path);
    header_.include("<memory>");
    header_ << "\n";
}

void
fwd_generator::finish_compilation_unit(ast::compilation_unit const& u)
{
}

void
fwd_generator::generate_forward_decl( ast::forward_declaration_ptr fwd )
{
}

void
fwd_generator::generate_constant( ast::constant_ptr c)
{
}

void
fwd_generator::generate_type_alias( ast::type_alias_ptr ta )
{
}

void
fwd_generator::generate_enum(ast::enumeration_ptr enum_)
{
}

void
fwd_generator::generate_struct(ast::structure_ptr struct_)
{
    header_.adjust_namespace(struct_);
    header_ << off << "struct " << qname(struct_) << ";";
}

void
fwd_generator::generate_exception(ast::exception_ptr exc)
{
    header_.adjust_namespace(exc);
    header_ << off << "class " << qname(exc) << ";";
    header_ << off << "using " << cpp_name(exc) << "_ptr = ::std::shared_ptr<"
                << cpp_name(exc) << ">;"
            << off << "using " << cpp_name(exc) << "_weak_ptr = ::std::weak_ptr<"
                << cpp_name(exc) << ">;\n";

}

void
fwd_generator::generate_interface(ast::interface_ptr iface)
{
    header_.adjust_namespace(iface);
    header_ << off     << "class " << qname(iface) << ";"
            << off     << "using " << cpp_name(iface)
            << "_ptr = ::std::shared_ptr< " << cpp_name(iface) << " >;"
            << off     << "using " << cpp_name(iface)
            << "_weak_ptr = ::std::weak_ptr< " << cpp_name(iface) << " >;";

    header_ << off     << "class " << qname(iface) << "_proxy;"
            << off     << "using " << cpp_name(iface)
            << "_prx = ::std::shared_ptr< " << cpp_name(iface) << "_proxy >;";
    header_ << "\n";
}

void
fwd_generator::generate_class(ast::class_ptr class_)
{
    header_.adjust_namespace(class_);
    header_ << "class " << qname(class_) << ";"
            << off << "using " << cpp_name(class_)
            << "_ptr = ::std::shared_ptr< " << cpp_name(class_) << " >;"
            << off << "using " << cpp_name(class_)
            << "_weak_ptr = ::std::weak_ptr< " << cpp_name(class_) << " >;\n";

    if (class_->has_functions()) {
        header_ << off << "class " << qname(class_) << "_proxy;"
                << off << "using " << cpp_name(class_)
                << "_prx = ::std::shared_ptr< " << cpp_name(class_) << "_proxy >;";
        header_ << "\n";
    }
}

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */
