/*
 * lua_generator.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: zmij
 */

#include <wire/wire2lua/lua_generator.hpp>
#include <wire/idl/syntax_error.hpp>

#include <iomanip>

namespace wire {
namespace idl {
namespace lua {

grammar::annotation_list const mapped_type::empty_annotations{};

generator::generator(ast::global_namespace_ptr ns, source_stream& out)
    : out_{out}, ns_{ns}, unit_{ns->current_compilation_unit()}
{
    out_ << off << "dprint2('Compilation unit " << unit_->name << "')\n\n";
}

generator::~generator()
{
    // TODO Auto-generated destructor stub
}

void
generator::finish_compilation_unit(ast::compilation_unit const& u)
{

}

void
generator::generate_forward_decl( ast::forward_declaration_ptr fwd )
{

}

void
generator::generate_constant( ast::constant_ptr c)
{

}

void
generator::generate_type_alias( ast::type_alias_ptr ta )
{
    out_ << off << "wire.types.alias('" << ta->get_qualified_name() << "', "
            << off(+1) << mapped_type{ta->alias()} << ")\n";
}

void
generator::generate_enum(ast::enumeration_ptr enum_)
{
    out_ << off << "wire.types.enum('" << enum_->get_qualified_name() << "', " << sb;
    int enum_val = 0;
    for (auto e : enum_->get_enumerators()) {
        if (e.second.is_initialized()) {
            try {
                enum_val = ::std::stoi(*e.second);
            } catch (::std::invalid_argument const& er) {
                // FIXME Try and calculate the value
//                throw grammar_error{ enum_->decl_position(),
//                    "Enumerator " + e.first + " in "
//                        + enum_->name() + " has an invalid initializer" };
                ::std::cerr << "Enumerator " << e.first << " in "
                            << enum_->name() + " has initializer that needs to be calculated\n";
            }
        }
        out_ << off << e.first << " = " << enum_val << ",";
        ++enum_val;
    }
    out_ << mod(-1) << "})\n";
}

void
generator::generate_struct(ast::structure_ptr struct_)
{
    if (!struct_->get_types().empty()) {
        for (auto t : struct_->get_types()) {
            generate_type_decl(t);
        }
    }
    out_ << off << "wire.types.structure('" << struct_->get_qualified_name() << "', " << sb;

    write_fields(struct_);
    process_annotations(struct_);

    out_ << mod(-1) << "})\n";
}

void
generator::generate_exception(ast::exception_ptr exc)
{
    if (!exc->get_types().empty()) {
        for (auto t : exc->get_types()) {
            generate_type_decl(t);
        }
    }
    out_ << off << "wire.types.exception('" << exc->get_qualified_name() << "', " << sb;
    out_ << off << "hash = '0x" << ::std::hex << ::std::setw(16) << ::std::setfill('0')
                << exc->get_name_hash() << ::std::dec << "',";

    write_fields(exc);

    out_ << mod(-1) << "})\n";
}

void
generator::generate_interface(ast::interface_ptr iface)
{
    if (!iface->get_types().empty()) {
        for (auto t : iface->get_types()) {
            generate_type_decl(t);
        }
    }
    out_ << off << "wire.types.interface('" << iface->get_qualified_name() << "', " << sb;
    write_functions(iface);
    out_ << mod(-1) << "})\n";
}

void
generator::generate_class(ast::class_ptr class_)
{
    if (!class_->get_types().empty()) {
        for (auto t : class_->get_types()) {
            generate_type_decl(t);
        }
    }
    out_ << off << "wire.types.class('" << class_->get_qualified_name() << "', " << sb;
    out_ << off << "hash = '0x" << ::std::hex << ::std::setw(16) << ::std::setfill('0')
                << class_->get_name_hash() << ::std::dec << "',";
    write_fields(class_);
    write_functions(class_);
    out_ << mod(-1) << "})\n";
}

void
generator::write_fields(ast::structure_ptr struct_)
{
    auto const& data_members = struct_->get_data_members();
    if (!data_members.empty()) {
        out_ << off << "fields = " << sb;
        for (auto dm : data_members) {
            out_ << off << sb
                << off << "name = '" << dm->name() << "',"
                << off << "type = " << mapped_type{dm->get_type()} << "," << eb;
        }
        out_ << eb;
    }
}

void
generator::write_functions(ast::interface_ptr iface)
{
    auto& funcs = iface->get_functions();
    if (!funcs.empty()) {
        out_ << off << "functions = " << sb;
        for (auto f : funcs) {
            out_ << off << f->name() << " = " << sb
                 << off     << "hash = '0x"
                     << ::std::hex << ::std::setw(8) << ::std::setfill('0')
                     << f->get_hash_32() << ::std::dec << "',";

            auto& params = f->get_params();
            if (!params.empty()) {
                out_ << off << "params = " << sb;
                for (auto const& p : params) {
                    out_ << off << sb
                        << off << "name = '" << p.second << "',"
                         << off << "type = " << mapped_type{p.first} << "," << eb;
                }
                out_ << eb;
            }
            if (f->get_return_type()->name() != ast::VOID) {
                out_ << off  << "ret = " << mapped_type{f->get_return_type()} << ",";
            }
            out_ << eb;
        }
        out_ << eb;
    }
}

void
generator::process_annotations(ast::entity_ptr e)
{
    auto ann = find(e->get_annotations(), annotations::LUA_FORMAT);
    if (ann != e->get_annotations().end()) {
        if (ann->arguments.empty()) {
            throw grammar_error{ e->decl_position(),
                "Lua format annotation must have an argument" };
        }
        out_ << off << "format = " << ann->arguments.front()->name << ",";
    }
}

} /* namespace lua */
} /* namespace idl */
} /* namespace wire */
