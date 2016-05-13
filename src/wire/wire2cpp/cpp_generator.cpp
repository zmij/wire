/*
 * cpp_generator.cpp
 *
 *  Created on: 27 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#include <wire/wire2cpp/cpp_generator.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <boost/filesystem.hpp>

#include <wire/types.hpp>
#include <wire/idl/syntax_error.hpp>
#include <wire/util/murmur_hash.hpp>

namespace wire {
namespace idl {
namespace cpp {

namespace {

struct type_mapping {
    ::std::string                     type_name;
    ::std::vector<::std::string>      headers;
};

::std::map< ::std::string, type_mapping > const wire_to_cpp = {
    /* Wire type      C++ Type                  Headers                                      */
    /*--------------+-------------------------+----------------------------------------------*/
    { "void",       { "void",                   {}                                          } },
    { "bool",       { "bool",                   {}                                          } },
    { "char",       { "char",                   {}                                          } },
    { "byte",       { "::std::int8_t",          {"<cstdint>"}                               } },
    { "int32",      { "::std::int32_t",         {"<cstdint>"}                               } },
    { "int64",      { "::std::int64_t",         {"<cstdint>"}                               } },
    { "octet",      { "::std::uint8_t",         {"<cstdint>"}                               } },
    { "uint32",     { "::std::int32_t",         {"<cstdint>"}                               } },
    { "uint64",     { "::std::int64_t",         {"<cstdint>"}                               } },
    { "float",      { "float",                  {}                                          } },
    { "double",     { "double",                 {}                                          } },
    { "string",     { "::std::string",          {"<string>"}                                } },
    { "uuid",       { "::boost::uuids::uuid",   {"<boost/uuid/uuid.hpp>",
                                                "<wire/encoding/detail/uuid_io.hpp>"}       } },

    { "variant",    { "::boost::variant",       {"<boost/variant.hpp>",
                                                "<wire/encoding/detail/variant_io.hpp>"}    } },
    { "optional",   { "::boost::optional",      {"<boost/optional.hpp>",
                                                "<wire/encoding/detail/optional_io.hpp>"}   } },
    { "sequence",   { "::std::vector",          {"<vector>",
                                                "<wire/encoding/detail/containers_io.hpp>"} } },
    { "array",      { "::std::array",           {"<array>",
                                                "<wire/encoding/detail/containers_io.hpp>"} } },
    { "dictionary", { "::std::map",             {"<map>",
                                                "<wire/encoding/detail/containers_io.hpp>"} } },

};

::std::size_t tab_width = 4;

void
strip_quotes(::std::string& str)
{
    if (!str.empty() && str.front() == '"') {
        str.erase(str.begin());
    }
    if (!str.empty() && str.back() == '"') {
        str.erase(--str.end());
    }
}

qname const root_interface {"::wire::core::object"};
qname const hash_value_type_name { "::wire::hash_value_type" };
qname const input_iterator_name { "::wire::encoding::incoming::const_iterator" };
qname const outgoing_name { "::wire::encoding::outgoing" };
qname const back_inserter { "::std::back_insert_iterator" };

}  /* namespace  */

struct tmp_pop_scope {
    qname&          scope;
    ::std::string   name;

    tmp_pop_scope(qname& s)
        : scope{s}, name{s.components.back()}
    {
        scope.components.pop_back();
    }
    ~tmp_pop_scope()
    {
        scope.components.push_back(name);
    }
};

struct tmp_push_scope {
    qname&          scope;
    ::std::string   name;

    tmp_push_scope(qname& s, ::std::string const& n)
        : scope{s}, name{n}
    {
        scope.components.push_back(name);
    }
    ~tmp_push_scope()
    {
        if (!scope.components.empty() && scope.components.back() == name) {
            scope.components.pop_back();
        }
    }
};

::std::ostream&
operator << (::std::ostream& os, offset const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        os << "\n";
        for (size_t i = 0; i < val.sz * tab_width; ++i) {
            os << " ";
        }
    }
    return os;
}

grammar::annotation_list const type_name_rules::empty_annotations{};

::std::ostream&
operator << (::std::ostream& os, relative_name const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        qname_search target = val.qn.search();
        for (auto c = val.current.begin;
                c != val.current.end && !target.empty() && *c == *target.begin;
                ++c, ++target);
        if (target.empty()) {
            os << val.qn;
        } else {
            os << target;
        }
    }
    return os;
}

::std::ostream&
operator << (::std::ostream& os, type_name_rules const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        if (auto pt = ast::dynamic_entity_cast< ast::parametrized_type >(val.type)) {
            ::std::string tmpl_name = wire_to_cpp.at(pt->name()).type_name;
            if (pt->name() == ast::ARRAY || pt->name() == ast::SEQUENCE || pt->name() == ast::DICTONARY) {
                auto ann = find(val.annotations, annotations::CPP_CONTAINER);
                if (ann != val.annotations.end()) {
                    if (ann->arguments.empty()) {
                        throw grammar_error(val.type->decl_position(), "Invalid cpp_container annotation");
                    }
                    tmpl_name = ann->arguments.front()->name;
                    strip_quotes(tmpl_name);
                }
            }
            os << tmpl_name << " <";
            for (auto p = pt->params().begin(); p != pt->params().end(); ++p) {
                if (p != pt->params().begin())
                    os << ", ";
                switch (p->which()) {
                    case ast::template_param_type::type:
                        os << type_name_rules{ val.current, ::boost::get< ast::type_ptr >(*p) };
                        break;
                    case ast::template_param_type::integral:
                        os << ::boost::get< ::std::string >(*p);
                        break;
                    default:
                        throw grammar_error(val.type->decl_position(),
                                "Unknown template parameter kind");
                }
            }
            os << " >";
            if (val.is_arg) {
                os << " const&";
            }
        } else {
            if (ast::type::is_built_in(val.type->get_qualified_name())) {
                os << wire_to_cpp.at( val.type->name() ).type_name;
                if (val.is_arg &&
                        (val.type->name() == ast::STRING || val.type->name() == ast::UUID))
                    os << " const&";
            } else if (auto ref = ast::dynamic_entity_cast< ast::reference >(val.type)) {
                os << relative_name{ val.current, val.type->get_qualified_name() } << "_prx";
            } else if (auto cl = ast::dynamic_type_cast< ast::class_ >(val.type)) {
                os << relative_name{ val.current, val.type->get_qualified_name() } << "_ptr";
            } else if (auto iface = ast::dynamic_type_cast< ast::interface >(val.type)) {
                os << relative_name{ val.current, val.type->get_qualified_name() } << "_ptr";
            } else {
                os << relative_name{ val.current, val.type->get_qualified_name() };
                if (val.is_arg) {
                    os << " const&";
                }
            }
        }
    }
    return os;
}


namespace fs = ::boost::filesystem;


generator::generator(generate_options const& opts, preprocess_options const& ppo,
        ast::global_namespace_ptr ns)
    : options_{opts}, ns_{ns}, unit_{ns->current_compilation_unit()},
      header_{ fs::path{unit_->name}.parent_path(),
            fs::path{ opts.header_include_dir }, ppo.include_dirs },
      source_{ fs::path{unit_->name}.parent_path(),
                fs::path{ opts.header_include_dir }, ppo.include_dirs },
      current_scope_{true}
{
    auto cwd = fs::current_path();

    fs::path origin(unit_->name);
    fs::path header_path{ opts.header_output_dir };
    fs::path source_path{ opts.source_output_dir };

    ::std::string origin_path = origin.parent_path().string();

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

    header_path /= origin.filename().stem().string() + ".hpp";

    if (source_path.empty()) {
        source_path = fs::current_path();
    }
    if (source_path.is_relative()) {
        source_path = fs::absolute(source_path);
    }
    if (!fs::exists(source_path)) {
        fs::create_directories(source_path);
    }
    source_path = fs::canonical(source_path);

    source_path /= origin.filename().stem().string() + ".cpp";

    header_.open(header_path);
    source_.open(source_path);
    header_ << "/* Hash for compilation unit is 0x"
            << ::std::hex << unit_->get_hash() << ::std::dec << " */\n";

    //------------------------------------------------------------------------
    ::std::set< ::std::string > standard_headers {"<memory>"};
    auto deps = unit_->external_dependencies();
    for (auto const& d : deps) {
        if (auto t = ast::dynamic_entity_cast< ast::type >(d)) {
            if (ast::type::is_built_in(d->get_qualified_name())) {
                auto const& tm = wire_to_cpp.at(d->name());
                if (!tm.headers.empty()) {
                    standard_headers.insert(tm.headers.begin(), tm.headers.end());
                }
            }
        }
    }

    // Collect custom headers and types from type aliases
    ast::entity_const_set type_aliases;
    for (auto const& e : unit_->entities) {
        e->collect_elements(type_aliases,
        [](ast::entity_const_ptr e){
            return ast::dynamic_entity_cast< ast::type_alias >(e).get();
        });
    }

    for (auto ta : type_aliases) {
        auto const& anns = ta->get_annotations();
        auto f = find(anns, annotations::CPP_CONTAINER);
        if (f != anns.end()) {
            if (f->arguments.size() < 2)
                throw grammar_error(ta->decl_position(),
                        "Invalid cpp_container annotation");
            ::std::string header = f->arguments[1]->name;
            strip_quotes(header);
            standard_headers.insert(header);
        }
    }
    standard_headers.insert("<wire/encoding/wire_io.hpp>");
    header_.include(standard_headers);
    //------------------------------------------------------------------------
    auto units = unit_->dependent_units();
    for (auto const& u : units) {
        if (!u->is_builtin()) {
            header_.include(fs::path{ u->name });
        }
    }

    if (unit_->has_exceptions()) {
        header_.include("<wire/errors/user_exception.hpp>");
    }
    if (unit_->has_interfaces()) {
        header_.include({
            "<wire/core/object.hpp>",
            "<wire/core/functional.hpp>",
            "<wire/core/proxy.hpp>"
        });

        source_.include({"<wire/core/reference.hpp>",
            "<wire/core/connection.hpp>",
            "<wire/core/detail/dispatch_request.hpp>",
            "<wire/core/invocation.hpp>",
            "<unordered_map>"
        });
    }
    if (unit_->has_classes()) {
        header_.include("<wire/encoding/detail/polymorphic_io.hpp>");
    }

    header_ << "\n";
    source_ << "\n";

    if (options_.dont_use_hashed_id) {
        ::std::cerr << "Don't use hashed type ids for marshalling\n";
    }

}

generator::~generator()
{
    adjust_scope(qname{}.search());
}

void generator::adjust_scope(ast::entity_ptr enum_)
{
    auto qn = enum_->get_qualified_name();
    adjust_scope(qn.search().scope());
}

void
generator::adjust_scope(qname_search const& target)
{
    qname_search current = current_scope_.search();

    auto c = current.begin;
    auto t = target.begin;
    for (; c != current.end && t != target.end && *c == *t; ++c, ++t);

    auto erase_start = c;
    ::std::deque<::std::string> to_close;
    for (; c != current.end; ++c) {
        to_close.push_front(*c);
    }
    for (auto const& ns : to_close) {
        header_ << h_off_ << "} /* namespace " << ns << " */";
        source_ << s_off_ << "} /* namespace " << ns << " */";
    }

    if (!to_close.empty()) {
        header_ << "\n";
        source_ << "\n";
    }


    current_scope_.components.erase(erase_start, current_scope_.components.end());

    bool space = t != target.end;
    for (; t != target.end; ++t) {
        header_ << h_off_ << "namespace " << *t << " {";
        source_ << s_off_ << "namespace " << *t << " {";
        current_scope_.components.push_back(*t);
    }
    if (space) {
        header_ << "\n";
        source_ << "\n";
    }
}

void
generator::pop_scope() {
    scope_stack_.pop_back();
    if (scope_stack_.empty()) {
        for (auto f : free_functions_) {
            f();
        }
        free_functions_.clear();
    }
}

::std::string
generator::constant_prefix(qname const& qn) const
{
    ::std::ostringstream os;
    for (auto c : qn.components) {
        ::std::transform(c.begin(), c.end(), c.begin(), toupper);
        os << c << "_";
    }
    return os.str();
}

source_stream&
generator::write_init(source_stream& os, offset& off, grammar::data_initializer const& init)
{
    switch (init.value.which()) {
        case 0: {
            os << ::boost::get< ::std::string >(init.value);
            break;
        }
        case 1: {
            grammar::data_initializer::initializer_list const& list =
                    ::boost::get<grammar::data_initializer::initializer_list>(init.value);
            if (list.size() < 3) {
                os << "{";
                for (auto p = list.begin(); p != list.end(); ++p) {
                    if (p != list.begin())
                        os << ", ";
                    write_init(os, off, *(*p));
                }
                os << "}";
            } else {
                os << "{" << ++off;
                for (auto p = list.begin(); p != list.end(); ++p) {
                    if (p != list.begin())
                        os << "," << off;
                    write_init(os, off, *(*p));
                }
                os << --off << "}";
            }
            break;
        }
    }
    return os;
}

source_stream&
generator::write_data_member(source_stream& os, offset const& off, ast::variable_ptr var)
{
    os << off << type_name(var->get_type(), var->get_annotations())
        << " " << var->name() << ";";
    return os;
}

void
generator::generate_dispatch_function_member(ast::function_ptr func)
{
    offset_guard hdr{h_off_};
    offset_guard src{s_off_};

    auto ann = find(func->get_annotations(), ast::annotations::SYNC);
    bool async_dispatch = ann == func->get_annotations().end();
    auto const& params = func->get_params();

    ::std::ostringstream formal_params;
    for (auto const& p : params) {
        formal_params << arg_type(p.first) << " " << p.second << ", ";
    }

    if (async_dispatch) {
        header_ << h_off_ << "/* Async dispatch */";
        ::std::ostringstream ret_cb_name;
        if (!func->is_void()) {
            ret_cb_name << func->name() << "_return_callback";
            header_ << h_off_ << "using " << ret_cb_name.str() << " = "
                << "::std::function< void("
                <<  arg_type(func->get_return_type(), func->get_annotations())
                << ") >;";
        } else {
            ret_cb_name << "::wire::core::functional::void_callback";
        }

        header_ << h_off_ << "virtual void"
                << h_off_ << func->name() << "(" << formal_params.str();
        if (!params.empty())
            header_ << (h_off_ + 2);
        header_ << ret_cb_name.str() << " __resp, "
                << (h_off_ + 2) << "::wire::core::functional::exception_callback __exception,"
                << (h_off_ + 2) << "::wire::core::current const& = ::wire::core::no_current)";
        if (func->is_const()) {
            header_ << " const";
        }
        header_ << " = 0;";
        header_ << h_off_ << "/*  ----8<    copy here   8<----"
                << h_off_ << "void"
                << h_off_ << func->name() << "(" << formal_params.str();
        if (!params.empty())
            header_ << (h_off_ + 2);
        header_ << ret_cb_name.str() << " __resp, "
                << (h_off_ + 2) << "::wire::core::functional::exception_callback __exception,"
                << (h_off_ + 2) << "::wire::core::current const& = ::wire::core::no_current)";
        if (func->is_const()) {
            header_ << " const";
        }
        header_ << " override;"
                << h_off_ << "    ---->8    end copy    >8----*/"
        ;
    } else {
        header_ << h_off_ << "/* Sync dispatch */"
                << h_off_ << "virtual "
                << type_name(func->get_return_type(), func->get_annotations())
                << h_off_ << func->name() << "(" << formal_params.str()
                << "::wire::core::current const& = ::wire::core::no_current)";
        if (func->is_const()) {
            header_ << " const";
        }
        header_ << " = 0;";
        header_ << h_off_ << "/*  ----8<    copy here   8<----"
                << h_off_ << type_name(func->get_return_type(), func->get_annotations())
                << h_off_ << func->name() << "(" << formal_params.str()
                << "::wire::core::current const& = ::wire::core::no_current)";
        if (func->is_const()) {
            header_ << " const";
        }
        header_ << " override;"
                << h_off_ << "    ---->8    end copy    >8----*/"
        ;
    }
    header_ << h_off_ << "/* dispatch function */"
            << h_off_ << "void"
            << h_off_ << "__" << func->name()
            << "(::wire::core::detail::dispatch_request const&, ::wire::core::current const&);\n";

    {
        tmp_pop_scope _pop{current_scope_};

        source_ << s_off_ << "void"
                << s_off_ << func->owner()->name() << "::__" << func->name()
                << "(::wire::core::detail::dispatch_request const& __req, ::wire::core::current const& __curr)"
                << s_off_ << "{";

        ++s_off_;
        // Unmarshal params
        ::std::ostringstream call_params;
        if (!params.empty()) {
            source_ << s_off_ << "auto __beg = __req.encaps_start;"
                    << s_off_ << "auto __end = __req.encaps_end;";
            for (auto p = params.begin(); p != params.end(); ++p) {
                source_ << s_off_ << type_name(p->first) << " " << p->second << ";";
                if (p != params.begin())
                    call_params << ", ";
                call_params << p->second;
            }
            source_ << s_off_ << "::wire::encoding::read(__beg, __end, " << call_params.str() << ");"
                    << s_off_ << "__req.encaps_start.incoming_encapsulation().read_indirection_table(__beg);";
        }

        ::std::ostringstream fcall;
        fcall << func->name() << "(" << call_params.str();
        if (!params.empty())
            fcall << ", ";
        if (async_dispatch) {
            // Generate callback
            source_ << s_off_ << fcall.str();
            source_ << ++s_off_ << "[__req](";
            if (!func->is_void()) {
                source_ <<  arg_type(func->get_return_type(), func->get_annotations())
                        << " _res";
            }
            source_ << ")"
                    << s_off_++ << "{";
            source_ << s_off_ << "::wire::encoding::outgoing __out{ __req.buffer->get_connector() };";
            if (!func->is_void()) {
                source_ << s_off_
                        << "::wire::encoding::write(::std::back_inserter(__out), _res);";
            }
            source_ << s_off_ << "__req.result(::std::move(__out));";
            source_ << --s_off_ << "}, __req.exception, __curr);";
            --s_off_;
        } else {
            fcall << "__curr)";
            source_ << s_off_ << "::wire::encoding::outgoing __out{ __req.buffer->get_connector() };";
            if (func->is_void()) {
                source_ << s_off_ << fcall.str() << ";";
            } else {
                source_ << s_off_ << "::wire::encoding::write(::std::back_inserter(__out), "
                        << fcall.str() << ");";
            }
            source_ << s_off_ << "__req.result(::std::move(__out));";
        }

        source_ << --s_off_ << "}\n";
    }
}

void
generator::generate_invocation_function_member(ast::function_ptr func)
{
    offset_guard hdr{h_off_};
    offset_guard src{s_off_};

    auto pfx = constant_prefix(func->owner()->get_qualified_name());

    ::std::ostringstream call_params;
    auto const& params = func->get_params();
    h_off_ += 2;
    for (auto const& p : params) {
        call_params << arg_type(p.first) << " " << p.second << "," << h_off_;
    }
    h_off_ -= 2;

    header_ << h_off_ << type_name(func->get_return_type(), func->get_annotations())
            << h_off_ << func->name() << "(" << call_params.str()
            << "::wire::core::context_type const& = ::wire::core::no_context);";

    ::std::ostringstream result_callback;
    if (func->is_void()) {
        result_callback << "::wire::core::functional::void_callback";
    } else {
        result_callback << func->name() << "_responce_callback";
        header_ << h_off_ << "using " << result_callback.str() << " = "
                << "::std::function< void( "
                << arg_type(func->get_return_type(), func->get_annotations())
                << " ) >;";
    }

    header_ << h_off_ << "void"
            << h_off_ << func->name() << "_async(";
    h_off_ += 2;
    header_ << call_params.str()
            << result_callback.str() << " _response,"
            << h_off_ << "::wire::core::functional::exception_callback _exception = nullptr,"
            << h_off_ << "::wire::core::functional::callback< bool > _sent        = nullptr,"
            << h_off_ << "::wire::core::context_type const&                      = ::wire::core::no_context,"
            << h_off_ << "bool                                      run_sync     = false);";
    h_off_ -= 2;

    header_ << h_off_ << "template < template< typename > class _Promise = ::std::promise >"
            << h_off_ << "auto"
            << h_off_ << func->name() << "_async(" << call_params.str()
            << "::wire::core::context_type const& _ctx = ::wire::core::no_context,"
            << h_off_ << "bool _run_async = false)";


    header_ << ++h_off_ << "-> decltype( ::std::declval< _Promise< "
                << type_name(func->get_return_type(), func->get_annotations())
                << " > >().get_future() )";
    header_ << (h_off_ - 1) << "{"
            << h_off_ << "auto promise = ::std::make_shared< _Promise <" <<
            type_name(func->get_return_type(), func->get_annotations()) << "> >();\n";

    header_ << h_off_ << func->name() << "_async(";
    ++h_off_;
    for (auto const& p : params) {
        header_ << h_off_ << p.second << ",";
    }
    if (func->is_void()) {
        header_ << h_off_ << "[promise]()"
                << h_off_ << "{ promise->set_value(); },";
    } else {
        header_ << h_off_ << "[promise](" << arg_type(func->get_return_type(), func->get_annotations()) << " res)"
                << h_off_ << "{ promise->set_value(res); },";
    }
    header_ << h_off_ << "[promise]( ::std::exception_ptr ex )"
            << h_off_ << "{ promise->set_exception(::std::move(ex)); },"
            << h_off_ << "nullptr, _ctx, _run_async";
    header_ << --h_off_ << ");\n";
    header_ << h_off_ << "return promise->get_future();";

    header_ << --h_off_ << "}\n";

    {
        // Sources
        tmp_pop_scope _pop{current_scope_};

        {
            offset_guard _f{s_off_};
            // Sync invocation
            source_ << s_off_ << type_name(func->get_return_type(), func->get_annotations())
                    << s_off_ << rel_name(func->owner()->name()) << "_proxy::" << func->name() << "(";
            s_off_ +=3;
            source_ << s_off_ << call_params.str()
                    << "::wire::core::context_type const& __ctx)";
            s_off_ -= 3;
            source_ << s_off_ << "{";
            source_ << ++s_off_ << "auto future = " << func->name() << "_async(";
            for (auto const& p : params) {
                source_ << p.second << ", ";
            }
            source_ << "__ctx, true);"
                    << s_off_ << "return future.get();";
            source_ << --s_off_ << "}\n";
        }
        {
            // Async invocation
            offset_guard _f{s_off_};
            source_ << s_off_ << "void"
                    << s_off_ << rel_name(func->owner()->name()) << "_proxy::" << func->name() << "_async(";
            s_off_ += 3;
            source_ << s_off_ << call_params.str()
                    << result_callback.str() << " _response,"
                    << s_off_ << "::wire::core::functional::exception_callback _exception,"
                    << s_off_ << "::wire::core::functional::callback< bool > _sent,"
                    << s_off_ << "::wire::core::context_type const& _ctx,"
                    << s_off_ << "bool _run_sync)"
                    << (s_off_ - 3) << "{";
            s_off_ -= 2;

            source_ << s_off_ << "make_invocation(wire_get_reference(),"
                    << (s_off_ + 2) << pfx << func->name() << ", _ctx,"
                    << (s_off_ + 2) << "&" << rel_name(func->owner()->name()) << "::" << func->name() << ","
                    << (s_off_ + 2) << "_response, _exception, _sent";
            if (!params.empty()) {
                source_ << (s_off_ + 2);
                for (auto const& p : params) {
                    source_ << ", " << p.second;
                }
            }
            source_ << ")(_run_sync);";

            --s_off_;
            source_ << s_off_ << "}\n";
        }
    }
}

void
generator::generate_forward_decl( ast::forward_declaration_ptr fwd )
{
    adjust_scope(fwd);
    switch (fwd->kind()) {
        case ast::forward_declaration::structure:
            header_ << h_off_ << "struct " << rel_name(fwd->get_qualified_name()) << ";";
            break;
        case ast::forward_declaration::interface:
            header_ << h_off_ << "class " << fwd->name() << "_proxy;"
                    << h_off_ << "using " << fwd->name() << "_prx = ::std::shared_ptr< "
                        << fwd->name() << "_proxy >;";
            ;
        case ast::forward_declaration::exception:
        case ast::forward_declaration::class_:
            header_ << h_off_ << "class " << fwd->name() << ";"
                    << h_off_ << "using " << fwd->name() << "_ptr = ::std::shared_ptr< "
                        << fwd->name() << ">;"
                    << h_off_ << "using " << fwd->name() << "_weak_ptr = ::std::weak_ptr< "
                        << fwd->name() << ">;"
            ;
        default:
            break;
    }
}

void
generator::generate_constant(ast::constant_ptr c)
{
    adjust_scope(c);

    header_ << h_off_ << type_name(c->get_type(), c->get_annotations())
            << " const " << c->name() << " = ";
    write_init(header_, h_off_, c->get_init());
    header_ << ";";
}

void
generator::generate_enum(ast::enumeration_ptr enum_)
{
    adjust_scope(enum_);
    header_ << h_off_++ << "enum ";
    if (enum_->constrained())
        header_ << "class ";
    header_ << type_name(enum_) << "{";
    for (auto e : enum_->get_enumerators()) {
        header_ << h_off_ << e.first;
        if (e.second.is_initialized()) {
            header_ << " = " << *e.second;
        }
        header_ << ",";
    }
    header_ << --h_off_ << "};\n";
}

void
generator::generate_type_alias( ast::type_alias_ptr ta )
{
    adjust_scope(ta);

    header_ << h_off_ << "using " << ta->name() << " = "
            <<  type_name(ta->alias(), ta->get_annotations()) << ";";
}

void
generator::generate_struct( ast::structure_ptr struct_ )
{
    adjust_scope(struct_);

    header_ << h_off_++ << "struct " << struct_->name() << " {";
    auto const& dm = struct_->get_data_members();
    {
        tmp_push_scope _push{current_scope_, struct_->name()};
        scope_stack_.push_back(struct_);

        for (auto t : struct_->get_types()) {
            generate_type_decl(t);
        }

        for (auto c : struct_->get_constants()) {
            generate_constant(c);
        }

        for (auto d : dm) {
            write_data_member(header_, h_off_, d);
        }

        if (!dm.empty()) {
            header_ << "\n" << h_off_ << "void"
                    << h_off_ << "swap(" << struct_->name() << "& rhs)"
                    << h_off_ << "{";

            header_ << ++h_off_ << "using ::std::swap;";
            for (auto d : dm) {
                header_ << h_off_ << "swap(" << d->name() << ", rhs." << d->name() << ");";
            }
            header_ << --h_off_ << "}";
        }

        header_ << --h_off_ << "};\n";
    }

    pop_scope();
    if (!dm.empty()) {
        if (scope_stack_.empty()) {
            generate_read_write(struct_);
            generate_comparison(struct_);
            generate_io(struct_);
        } else {
            free_functions_.push_back(
                    ::std::bind(&generator::generate_read_write, this, struct_));
            free_functions_.push_back(
                    ::std::bind(&generator::generate_comparison, this, struct_));
            free_functions_.push_back(
                    ::std::bind(&generator::generate_io, this, struct_));
        }
    }
}

void
generator::generate_read_write( ast::structure_ptr struct_)
{
    auto const& dm = struct_->get_data_members();
    header_ << h_off_ << "template < typename OutputIterator >"
            << h_off_ << "void"
            << h_off_ << "wire_write(OutputIterator o, "
            <<  type_name(struct_) << " const& v)"
            << h_off_ << "{";

    header_ << ++h_off_ << "::wire::encoding::write(o";
    for (auto d : dm) {
        header_ << ", v." << d->name();
    }
    header_ << ");";
    header_ << --h_off_ << "}\n";

    header_ << h_off_ << "template < typename InputIterator >"
            << h_off_ << "void"
            << h_off_ << "wire_read(InputIterator& begin, InputIterator end, "
            << type_name(struct_) << "& v)"
            << h_off_ << "{";

    header_ << ++h_off_ << type_name(struct_) << " tmp;"
            << h_off_ << "::wire::encoding::read(begin, end";
    for (auto d : dm) {
        header_ << ", tmp." << d->name();
    }
    header_ << ");"
            << h_off_ << "v.swap(tmp);";
    header_ << --h_off_ << "}\n";
}

void
generator::generate_member_read_write( ast::structure_ptr struct_,
        ast::structure_const_ptr parent, bool ovrde)
{
    offset_guard hdr{h_off_};
    offset_guard src{s_off_};

    auto const& data_members = struct_->get_data_members();
    ::std::ostringstream qn_os;
    qn_os << struct_->get_qualified_name();
    auto qn_str = qn_os.str();

    header_ << ++h_off_ << (( parent || ovrde ) ? "" : "virtual ") << "void"
            << h_off_ << "__wire_write(output_iterator o) const"
                    << (( parent || ovrde ) ? " override" : "") << ";";

    header_ << h_off_ << (( parent || ovrde ) ? "" : "virtual ") << "void"
            << h_off_ << "__wire_read(input_iterator& begin, input_iterator end, "
                            "bool read_head = true)"
                    << (( parent || ovrde ) ? " override" : "") << ";";

    // Source
    tmp_pop_scope _pop{current_scope_};
    //------------------------------------------------------------------------
    //  Wire write member function
    //------------------------------------------------------------------------
    source_ << s_off_ << "void"
            << s_off_ << rel_name(struct_)
            << "::__wire_write(output_iterator o) const"
            << s_off_ << "{";

    ++s_off_;

    source_ << s_off_ << "auto encaps = o.encapsulation();";

    ::std::string flags = "::wire::encoding::segment_header::none";
    if (!parent) {
        flags = "::wire::encoding::segment_header::last_segment";
    }

    ::std::string type_id_func = options_.dont_use_hashed_id ? "wire_static_type_id"
            : (qn_str.size() > sizeof(hash_value_type)) ?
                    "wire_static_type_id_hash" : "wire_static_type_id";
    source_ << s_off_ << "encaps.start_segment(" << type_id_func << "(), " << flags << ");";

    if (!data_members.empty()) {
        source_ << s_off_ << "::wire::encoding::write(o";
        for (auto dm : data_members) {
            source_ << ", " << dm->name();
        }
        source_ << ");";
    }

    source_ << s_off_ << "encaps.end_segment();";

    if (parent) {
        source_ << s_off_ << parent->name() << "::__wire_write(o);";
    }

    source_ << --s_off_ << "}\n";

    //------------------------------------------------------------------------
    //  Wire read member function
    //------------------------------------------------------------------------
    source_ << s_off_ << "void"
            << s_off_ << rel_name(struct_)
            << "::__wire_read(input_iterator& begin, input_iterator end, "
                    "bool read_head)"
            << s_off_ << "{";
    ++s_off_;

    source_ << s_off_ << "auto encaps = begin.incoming_encapsulation();";
    source_ << s_off_++ << "if (read_head) {";
    source_ << s_off_ <<    "::wire::encoding::segment_header seg_head;"
            << s_off_ <<    "encaps.read_segment_header(begin, end, seg_head);";
    source_ << s_off_ << "::wire::encoding::check_segment_header< " << struct_->name()
                            << " >(seg_head);";
    source_ << --s_off_ << "}\n";

    if (!data_members.empty()) {
        source_ << s_off_ << "::wire::encoding::read(begin, end";
        for (auto dm : data_members) {
            source_ << ", " << dm->name();
        }
        source_ << ");";
    }

    if (parent) {
        source_ << s_off_ << parent->name() << "::__wire_read(begin, encaps.end(), true);";
    }

    source_ << --s_off_ << "}\n";
}

void
generator::generate_comparison( ast::structure_ptr struct_)
{
    auto f = find(struct_->get_annotations(), annotations::GENERATE_CMP);
    if (f == struct_->get_annotations().end())
        return;

    auto const& dm = struct_->get_data_members();
    header_ << h_off_ << "inline bool"
            << h_off_ << "operator == (" << arg_type(struct_) << " lhs, "
                    << arg_type(struct_) << " rhs)"
            << h_off_ << "{";

    ++h_off_;
    if (!dm.empty()) {
        header_ << h_off_ << "return ";
        for (auto d = dm.begin(); d != dm.end(); ++d) {
            if (d != dm.begin()) {
                header_ << " && " << (h_off_ + 2);
            }
            header_ << "lhs." << (*d)->name() << " == rhs." << (*d)->name();
        }
        header_ << ";";
    } else {
        header_ << h_off_ << "return &lhs == &rhs;";
    }

    header_ << --h_off_ << "}";

    header_ << h_off_ << "inline bool"
            << h_off_ << "operator != (" << arg_type(struct_) << " lhs, "
                    << arg_type(struct_) << " rhs)"
            << h_off_ << "{"
            << (h_off_ + 1) << "return !(lhs == rhs);"
            << h_off_ << "}";

    header_ << h_off_ << "inline bool"
            << h_off_ << "operator < (" << arg_type(struct_) << " lhs, "
                    << arg_type(struct_) << " rhs)"
            << h_off_ << "{";

    ++h_off_;
    if (!dm.empty()) {
        header_ << h_off_ << "return ";
        for (auto d = dm.begin(); d != dm.end(); ++d) {
            if (d != dm.begin()) {
                header_ << " && " << (h_off_ + 2);
            }
            header_ << "lhs." << (*d)->name() << " < rhs." << (*d)->name();
        }
        header_ << ";";
    } else {
        header_ << h_off_ << "return &lhs < &rhs;";
    }

    header_ << --h_off_ << "}\n";
}

void
generator::generate_io( ast::structure_ptr struct_)
{
    auto f = find(struct_->get_annotations(), annotations::GENERATE_IO);
    if (f == struct_->get_annotations().end())
        return;

    auto const& data_members = struct_->get_data_members();
    header_ << h_off_ << "::std::ostream&"
            << h_off_ << "operator << ( ::std::ostream& os, "
                    << arg_type(struct_) << " val );\n";

    source_ << s_off_ << "::std::ostream&"
            << s_off_ << "operator << ( ::std::ostream& os, "
                    << arg_type(struct_) << " val )"
            << s_off_ << "{";
    {
        if (data_members.empty()) {
            source_ << (s_off_ + 1) << "return os;";
        } else {
            offset_guard src{s_off_};
            source_ << ++s_off_ << "::std::ostream::sentry s{os};"
                    << s_off_ << "if (s) {";
            ++s_off_;
            source_ << s_off_ << "os << '{' ";
            for (auto dm = data_members.begin(); dm != data_members.end(); ++dm) {
                if (dm != data_members.begin())
                    source_ << " << \" \"" << (s_off_ + 1);
                source_ << "<< val." << (*dm)->name();
            }
            source_ << " << '}';";
            source_ << --s_off_ << "}"
                    << s_off_ << "return os;";
        }
    }
    source_ << s_off_ << "}";
}

::std::string
generator::generate_type_id_funcs(ast::entity_ptr elem)
{
    offset_guard hdr{h_off_};
    offset_guard src{s_off_};
    header_ << h_off_++ << "public:";

    header_ << h_off_ << "static ::std::string const&"
            << h_off_ << "wire_static_type_id();";

    header_ << h_off_ << "static " << rel_name(hash_value_type_name)
            << h_off_ << "wire_static_type_id_hash();";

    tmp_pop_scope _pop{current_scope_};
    // Source
    auto eqn = elem->get_qualified_name();
    auto pfx = constant_prefix(eqn);
    ::std::ostringstream qnos;
    qnos << eqn;
    auto eqn_str = qnos.str();
    source_ << s_off_ << "/* data for " << eqn
                << " 0x" << ::std::hex << elem->get_hash() << ::std::dec << " */";
    source_ << s_off_++ << "namespace {\n";
    source_ << s_off_ << "::std::string const " << pfx << "TYPE_ID = \"" << eqn_str
            << "\";";
    source_ << s_off_ << rel_name(hash_value_type_name) << " const " << pfx << "TYPE_ID_HASH = 0x"
            << ::std::hex << hash::murmur_hash(eqn_str) << ::std::dec << ";";
    source_ << "\n" << --s_off_ << "} /* namespace for " << eqn << " */\n";

    source_ << s_off_ << "::std::string const&"
            << s_off_ << rel_name(eqn) << "::wire_static_type_id()"
            << s_off_ << "{";
    source_ << ++s_off_ << "return " << pfx << "TYPE_ID;";
    source_ << --s_off_ << "}\n";

    source_ << s_off_ << rel_name(hash_value_type_name)
            << s_off_ << rel_name(eqn) << "::wire_static_type_id_hash()"
            << s_off_ << "{";
    source_ << ++s_off_ << "return " << pfx << "TYPE_ID_HASH;";
    source_ << --s_off_ << "}\n";

    return eqn_str;
}

void
generator::generate_wire_functions(ast::interface_ptr iface)
{
    offset_guard hdr{h_off_};
    offset_guard src{s_off_};
    auto eqn = iface->get_qualified_name();

    header_ << h_off_++ << "public:";

    header_ << h_off_ << "bool"
            << h_off_ << "wire_is_a(::std::string const&,"
            << (h_off_+1) << "::wire::core::current const& = ::wire::core::no_current) const override;\n"
            << h_off_ << "::std::string const&"
            << h_off_ << "wire_type(::wire::core::current const& = ::wire::core::no_current) const override;\n"
            << h_off_ << "type_list const&"
            << h_off_ << "wire_types(::wire::core::current const& = ::wire::core::no_current) const override;\n"
            << (h_off_ - 1) << "protected:"
            << h_off_ << "bool"
            << h_off_ << "__wire_dispatch(::wire::core::detail::dispatch_request const&, ::wire::core::current const&,"
            << (h_off_ +2) << "dispatch_seen_list&, bool throw_not_found) override;"
    ;

    tmp_pop_scope _pop{current_scope_};

    //------------------------------------------------------------------------
    // data
    auto pfx = constant_prefix(eqn);
    {
        auto const& funcs = iface->get_functions();
        source_ << s_off_ << "namespace { /*    Type ids and dispathc map for "
                        << iface->get_qualified_name() << "  */\n";
        offset_guard cn{s_off_};
        ++s_off_;
        for (auto f : funcs) {
            source_ << s_off_ << "::std::string const " << pfx << f->name()
                    << " = \"" << f->name() << "\";";
        }
        if (!funcs.empty())
            source_ << "\n";
        source_ << s_off_ << "using " << pfx << "dispatch_func ="
                << (s_off_ + 1) << " void(" << rel_name(eqn)
                << "::*)(::wire::core::detail::dispatch_request const&, ::wire::core::current const&);\n";

        source_ << s_off_ << "::std::unordered_map< ::std::string, " << pfx << "dispatch_func >"
                << " const " << pfx << "dispatch_map {";

        ++s_off_;
        for (auto f : funcs) {
            source_ << s_off_ << "{ " << pfx << f->name() << ", &" << rel_name(eqn)
                    << "::__" << f->name() << " },";
        }

        source_ << --s_off_ << "};\n";

        source_ << s_off_ << rel_name(eqn) << "::type_list const "
                << pfx << "TYPE_IDS = {";
        ++s_off_;
        source_ << s_off_ << root_interface << "::wire_static_type_id(),";
        ast::interface_list ancestors;
        iface->collect_ancestors(ancestors);
        for (auto a : ancestors) {
            source_ << s_off_ << rel_name(a->get_qualified_name()) << "::wire_static_type_id(),";
        }
        source_ << s_off_ << rel_name(iface->get_qualified_name()) << "::wire_static_type_id(),";
        source_ << --s_off_ << "};";
        source_ << --s_off_ << "} /* namespace */\n";
    }



    //------------------------------------------------------------------------
    // is_a
    source_ << s_off_ << "bool"
            << s_off_ << rel_name(eqn) << "::wire_is_a(::std::string const& name,"
            << (s_off_+1) << "::wire::core::current const&) const"
            << s_off_ << "{";
    ++s_off_;

    source_ << s_off_ << "for (auto const& t : " << pfx << "TYPE_IDS) {"
            << (s_off_ + 1) << "if (t == name) return true;"
            << s_off_ << "}"
            << s_off_ << "return false;";
    source_ << --s_off_ << "}";

    //------------------------------------------------------------------------
    // type
    source_ << s_off_ << "::std::string const&"
            << s_off_ << rel_name(eqn) << "::wire_type(::wire::core::current const&) const"
            << s_off_ << "{"
            << (s_off_ + 1) << "return wire_static_type_id();"
            << s_off_ << "}";

    //------------------------------------------------------------------------
    // types
    source_ << s_off_ << rel_name(eqn) << "::type_list const&"
            << s_off_ << rel_name(eqn) << "::wire_types(::wire::core::current const&) const"
            << s_off_ << "{"
            << (s_off_ + 1) << "return " << pfx << "TYPE_IDS;"
            << s_off_ << "}";

    //------------------------------------------------------------------------
    // the dispatch function
    source_ << s_off_ << "bool"
            << s_off_ << rel_name(eqn) << "::__wire_dispatch(::wire::core::detail::dispatch_request const& req,"
            << (s_off_ + 1) << "::wire::core::current const& c,"
            << (s_off_ + 1) << "dispatch_seen_list& seen, bool throw_not_found)"
            << s_off_ << "{";
    source_ << ++s_off_ << "if (seen.count(wire_static_type_id_hash())) return false;"
            << s_off_ << "seen.insert(wire_static_type_id_hash());";
    source_ << s_off_ << "if (c.operation.type() == ::wire::encoding::operation_specs::name_string) {";
    source_ << ++s_off_;
    source_ << s_off_ << "auto f = " << pfx << "dispatch_map.find(c.operation.name());"
            << s_off_ << "if (f != " << pfx << "dispatch_map.end()) {"
            << (s_off_ + 1) << "(this->*f->second)(req, c);"
            << (s_off_ + 1) << "return true;"
            << s_off_ << "}";
    source_ << s_off_ << "bool res = ";
    ++s_off_;
    auto const& ancestors = iface->get_ancestors();
    if (!ancestors.empty()) {
        for (auto a = ancestors.begin(); a != ancestors.end(); ++a) {
            if (a != ancestors.begin())
                source_ << " ||" << s_off_;
            source_ << rel_name((*a)->get_qualified_name()) << "::__wire_dispatch(req, c, seen, false)";
        }
    } else {
        source_ << rel_name(root_interface) << "::__wire_dispatch(req, c, seen, false)";
    }
    source_ << ";";
    --s_off_;
    source_ << s_off_ << "if (!res && throw_not_found)"
            << (s_off_ + 1) << "throw ::wire::errors::no_operation{"
            << (s_off_ + 2) << "wire_static_type_id(), \"::\", c.operation.name()};";
    source_ << s_off_ << "return res;";
    source_ << --s_off_ << "} else {"
            << (s_off_ + 1) << "throw ::wire::errors::no_operation{"
            << (s_off_ + 2) << "wire_static_type_id(), \"::\", c.operation.name()};";
    source_ << s_off_ << "}";
    source_ << --s_off_ << "}\n";
}

void
generator::generate_exception(ast::exception_ptr exc)
{
    adjust_scope(exc);

    source_ << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-"
            << s_off_ << "//    Exception " << exc->get_qualified_name()
            << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-";

    static const qname root_exception {"::wire::errors::user_exception"};

    qname parent_name = exc->get_parent() ?
            exc->get_parent()->get_qualified_name() : root_exception;

    header_ << h_off_ << "class " << exc->name()
            << " : public " << rel_name(parent_name) << " {";

    auto const& data_members = exc->get_data_members();
    ::std::string qn_str;
    {
        tmp_push_scope _push{current_scope_, exc->name()};
        scope_stack_.push_back(exc);

        if (!exc->get_types().empty()) {
            header_ << h_off_++ << "public:";
            for (auto t : exc->get_types()) {
                generate_type_decl(t);
            }
            --h_off_;
        }
        if (!exc->get_constants().empty()) {
            header_ << h_off_++ << "public:";
            for (auto c : exc->get_constants()) {
                generate_constant(c);
            }
            --h_off_;
        }

        // Constructors
        header_ << h_off_++ << "public:";

        ::std::ostringstream members_init;
        if (!data_members.empty()) {
            for (auto dm : data_members) {
                members_init << ", " << dm->name() << "{}";
            }
        }

        //@{ Default constructor
        header_ << h_off_ << "/* default constructor, for use in factories */"
                << h_off_ << exc->name() << "() : " << rel_name(parent_name) << "{}"
                << members_init.str() << " {}";
        //@}

        header_ << h_off_ << "/* templated constructor to format a ::std::runtime_error message */"
                << h_off_ << "template < typename ... T >"
                << h_off_ << exc->name() << "(T const& ... args) : "
                << rel_name(parent_name) << "(args ...)"
                << members_init.str() << "{}";

        // TODO constructors with data members variations
        if (!data_members.empty()) {
            ::std::ostringstream args;
            ::std::ostringstream init;
            ::std::ostringstream msg;

            ::std::deque<::std::string> rest;
            for (auto dm : data_members) {
                rest.push_back(dm->name() + "{}");
            }
            for (auto p = data_members.begin(); p != data_members.end(); ++p) {
                if (p != data_members.begin()) {
                    args << ", ";
                    init << "," << (h_off_ + 1) << "  ";
                    msg << ", ";
                }
                args << arg_type((*p)->get_type()) << " " << (*p)->name() << "_";
                init << (*p)->name() << "{" << (*p)->name() << "_}";
                msg << (*p)->name() << "_";

                rest.pop_front();

                header_ << h_off_ << exc->name() << "(" << args.str() << ")";
                header_ << ++h_off_ << ": " << rel_name(parent_name)
                        << "(wire_static_type_id(), " << msg.str() << "), "
                        << h_off_ << "  " << init.str();
                for (auto const& r : rest) {
                    header_ << "," << h_off_ << "  " << r;
                }
                header_ << " {}";
                --h_off_;
            }
        }
        --h_off_;

        if (!data_members.empty()) {
            header_ << h_off_++ << "public:";
            for (auto dm : data_members) {
                header_ << h_off_
                        << type_name(dm->get_type()) << " " << dm->name() << ";";
            }
            --h_off_;
        }

        qn_str = generate_type_id_funcs(exc);
        {
            // Member functions
            generate_member_read_write(exc, exc->get_parent());
            header_ << h_off_ << "::std::exception_ptr"
                    << h_off_ << "make_exception_ptr() override"
                    << h_off_ << "{ return ::std::make_exception_ptr(*this); }";
        }

        header_ << --h_off_ << "};\n";
    }

    // TODO to outer scope
    header_ << h_off_ << "using " << exc->name() << "_ptr = ::std::shared_ptr<"
                << exc->name() << ">;"
            << h_off_ << "using " << exc->name() << "_weak_ptr = ::std::weak_ptr<"
                << exc->name() << ">;\n";

    // Source
    //------------------------------------------------------------------------
    //  Factory initializer for the exception
    //------------------------------------------------------------------------
    source_ << s_off_ << "namespace {"
            << (s_off_ + 1) << "::wire::errors::user_exception_factory_init< "
            << rel_name(exc->get_qualified_name()) << " > const "
            << constant_prefix(exc->get_qualified_name()) << "_factory_init;"
            << s_off_ << "}";
    pop_scope();
}

void
generator::generate_dispatch_interface(ast::interface_ptr iface)
{
    adjust_scope(iface);

    source_ << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-"
            << s_off_ << "//    Dispatch interface for " << iface->get_qualified_name()
            << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-";

    header_ << h_off_ << "/**"
            << h_off_ << " *    Dispatch interface for " << iface->get_qualified_name()
            << h_off_ << " */"
            << h_off_ << "class " << iface->name();

    auto const& ancestors = iface->get_ancestors();
    if (!ancestors.empty()) {
        header_ << ++h_off_ << ": ";
        for ( auto a = ancestors.begin(); a != ancestors.end(); ++a ) {
            if (a != ancestors.begin())
                header_ << "," << h_off_ << "  ";
            header_ << "public virtual " << rel_name((*a)->get_qualified_name());
        }
        --h_off_;
    } else {
        header_ << " : public virtual " << rel_name(root_interface);
    }
    header_ << " {";

    tmp_push_scope _push{current_scope_, iface->name()};
    scope_stack_.push_back(iface);

    if (!iface->get_types().empty()) {
        header_ << h_off_++ << "public:";
        for (auto t : iface->get_types()) {
            generate_type_decl(t);
        }
        --h_off_;
    }
    if (!iface->get_constants().empty()) {
        header_ << h_off_++ << "public:";
        for (auto c : iface->get_constants()) {
            generate_constant(c);
        }
        --h_off_;
    }
    {
        // Constructor
        header_ << h_off_ << "public:";
        header_ << ++h_off_ << iface->name() << "()";
        header_ << ++h_off_ << ": " << rel_name(root_interface) << "()";
        ast::interface_list anc;

        iface->collect_ancestors(anc, [](ast::entity_const_ptr){ return true; });

        for (auto a : anc) {
            header_ << "," << h_off_ << "  "
                    << rel_name(a->get_qualified_name()) << "()";
        }

        header_ << " {}\n";
        h_off_ -= 2;
    }

    auto qn_str = generate_type_id_funcs(iface);
    generate_wire_functions(iface);
    auto const& funcs = iface->get_functions();
    if (!funcs.empty()) {
        header_ << h_off_++ << "public:";
        // Dispatch methods
        for (auto f : funcs) {
            generate_dispatch_function_member(f);
        }
        --h_off_;
    }

    header_ << --h_off_ << "};\n";
    pop_scope();
}

void
generator::generate_proxy_interface(ast::interface_ptr iface)
{
    static const qname base_proxy {"::wire::core::object_proxy"};
    static const qname ref_ptr    {"::wire::core::reference_ptr"};

    source_ << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-"
            << s_off_ << "//    Proxy interface for " << iface->get_qualified_name()
            << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-";

    header_ << h_off_ << "/**"
            << h_off_ << " *    Proxy interface for " << iface->get_qualified_name()
            << h_off_ << " */"
            << h_off_ << "class " << iface->name() << "_proxy : "
            << (h_off_ + 1) << "public virtual ::wire::core::proxy< " << iface->name();


    auto const& ancestors = iface->get_ancestors();
    if (!ancestors.empty()) {
        h_off_ += 2;
        for ( auto a : ancestors ) {
            auto qn = a->get_qualified_name();
            qn.components.back() += "_proxy";
            header_ << "," << h_off_ << rel_name(qn);
        }
        h_off_ -= 2;
    } else {
        header_ << ", " << rel_name(base_proxy);
    }
    header_ << "> {"
            << h_off_ << "public:";

    tmp_push_scope _push{current_scope_, iface->name() + "_proxy"};
    scope_stack_.push_back(iface);
    {
        offset_guard hdr{h_off_};
        // TODO Constructors
        header_ << ++h_off_ << iface->name() << "_proxy ()"
                << (h_off_ + 2) << ": " << rel_name(base_proxy) << "{} {}"
                << h_off_ << iface->name() << "_proxy (" << rel_name(ref_ptr)
                    << " _ref)"
                << (h_off_ + 2) << ": " << rel_name(base_proxy) << "{_ref} {}";
        ;
    }
    {
        offset_guard hdr{h_off_};
        header_ << h_off_++ << "public:"
                << h_off_ << "static ::std::string const&"
                << h_off_ << "wire_static_type_id();";

        tmp_pop_scope _t_id{current_scope_};
        source_ << s_off_ << "::std::string const&"
                << s_off_ << rel_name(iface->get_qualified_name())
                << "_proxy::wire_static_type_id()"
                << s_off_ << "{"
                << (s_off_ + 1) << "return " << rel_name(iface->get_qualified_name())
                        << "::wire_static_type_id();"
                << s_off_ << "}\n"
        ;
    }
    auto const& funcs = iface->get_functions();
    if (!funcs.empty()) {
        offset_guard hdr{h_off_};
        header_ << h_off_++ << "public:";
        for (auto f : funcs) {
            generate_invocation_function_member(f);
        }
    }
    header_ << h_off_ << "};\n";

    pop_scope();
}

void
generator::generate_interface(ast::interface_ptr iface)
{
    generate_dispatch_interface(iface);
    generate_proxy_interface(iface);

    header_ << h_off_ << "using " << iface->name()
            << "_ptr = ::std::shared_ptr< " << iface->name() << " >;";
    header_ << h_off_ << "using " << iface->name()
            << "_weak_ptr = ::std::weak_ptr< " << iface->name() << " >;";

    header_ << h_off_ << "using " << iface->name()
            << "_prx = ::std::shared_ptr< " << iface->name() << "_proxy >;";
    header_ << "\n";
}

void
generator::generate_class(ast::class_ptr class_)
{
    offset_guard hdr{h_off_};
    offset_guard src{s_off_};
    qname cqn = class_->get_qualified_name();
    adjust_scope(class_);
    source_ << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-"
            << s_off_ << "//    Implementation for class " << cqn
            << s_off_ << "//" << ::std::setw(77) << ::std::setfill('-') << "-";

    header_ << h_off_ << "/**"
            << h_off_ << " *    Class " << cqn
            << h_off_ << " */"
            << h_off_ << "class " << class_->name();
    auto const& ancestors = class_->get_ancestors();
    auto const& data_members = class_->get_data_members();
    auto parent = class_->get_parent();
    ::std::string qn_str;
    if (parent) {
        header_ << (h_off_ + 1) << ": public " << rel_name(parent);
    }
    if (!ancestors.empty()) {
        ++h_off_;
        if (parent) {
            header_ << "," << h_off_ << "  ";
        } else {
            header_ << h_off_ << ": ";
        }
        for (auto a = ancestors.begin(); a != ancestors.end(); ++a) {
            if (a != ancestors.begin())
                header_ << "," << h_off_ << "  ";
            header_ << "public virtual " << rel_name(*a);
        }
        --h_off_;
    } else if (class_->has_functions()) {
        ++h_off_;
        if (parent) {
            header_ << "," << h_off_ << "  public virtual " << rel_name(root_interface);
        } else {
            header_ << h_off_ << ": public virtual " << rel_name(root_interface);
        }
        --h_off_;
    }
    header_ << " {";
    {
        tmp_push_scope _push{current_scope_, class_->name()};
        scope_stack_.push_back(class_);

        header_ << h_off_++ << "public:";
        header_ << h_off_ << "using wire_root_type = "
                << rel_name(class_->root_class()->get_qualified_name()) << ";";
        if (!parent) {
            header_ << h_off_ << "using input_iterator = " << rel_name(input_iterator_name) << ";"
                    << h_off_ << "using output_iterator = " << rel_name(back_inserter)
                        << "< " << rel_name(outgoing_name) << " >;";
        }
        if (!class_->get_types().empty()) {
            for (auto t : class_->get_types()) {
                generate_type_decl(t);
            }
        }
        --h_off_;
        if (!class_->get_constants().empty()) {
            header_ << h_off_++ << "public:";
            for (auto c : class_->get_constants()) {
                generate_constant(c);
            }
            --h_off_;
        }

        {
            ::std::ostringstream members_init;
            if (!data_members.empty()) {
                for (auto dm = data_members.begin(); dm != data_members.end(); ++dm) {
                    if (dm != data_members.begin())
                        members_init << ", ";
                    members_init << (*dm)->name() << "{}";
                }
            }
            // Constructor
            header_ << h_off_++ << "public:";
            header_ << h_off_ << class_->name() << "()";
            bool need_comma {false};
            if (parent) {
                header_ << (h_off_ + 1) << ": " << rel_name(parent) << "{}";
                need_comma = true;
            }
            if (class_->is_abstract()) {
                if (need_comma) {
                    header_ << "," << (h_off_ + 1) << "  ";
                } else {
                    header_ << (h_off_ + 1) << ": ";
                }
                header_ << rel_name(root_interface) << "{}";
                for (auto a : ancestors) {
                    header_ << "," << (h_off_ + 1)
                            << "  " << rel_name(a) << "{}";
                }
                need_comma = true;
            }
            if (!data_members.empty()) {
                if (need_comma) {
                    header_ << "," << (h_off_ + 1) << "  ";
                } else {
                    header_ << (h_off_ + 1) << ": ";
                }
                header_ << members_init.str();
            }

            header_ << " {}";
            --h_off_;
        }
        {
            // Destuctor
            header_ << (h_off_ + 1) << "virtual ~" << class_->name() << "() {}";
        }
        qn_str = generate_type_id_funcs(class_);
        {
            // Member functions
            generate_member_read_write(class_, parent, false);
        }

        if (class_->is_abstract()) {
            generate_wire_functions(class_);
            auto const& funcs = class_->get_functions();
            if (!funcs.empty()) {
                header_ << h_off_++ << "public:";
                // Dispatch methods
                for (auto f : funcs) {
                    generate_dispatch_function_member(f);
                }
                --h_off_;
            }
        } else {
            // factory initializer
            //------------------------------------------------------------------------
            //  Factory initializer for the class
            //------------------------------------------------------------------------
            source_ << s_off_ << "namespace {"
                    << (s_off_ + 1) << "::wire::encoding::detail::auto_object_factory_init< "
                    << rel_name(cqn) << " > const "
                    << constant_prefix(cqn) << "_factory_init;"
                    << s_off_ << "}";
        }
        {
            // Data members
            if (!data_members.empty()) {
                header_ << h_off_++ << "public:";
                for (auto dm : data_members) {
                    header_ << h_off_
                            << type_name(dm->get_type()) << " " << dm->name() << ";";
                }
                --h_off_;
            }
        }

        header_ << h_off_ << "};\n";
    }
    pop_scope();

    //------------------------------------------------------------------------
    //  Type aliases for class pointers
    //------------------------------------------------------------------------
    header_ << h_off_ << "using " << class_->name()
            << "_ptr = ::std::shared_ptr< " << class_->name() << " >;";
    header_ << h_off_ << "using " << class_->name()
            << "_weak_ptr = ::std::weak_ptr< " << class_->name() << " >;\n";

    if (class_->has_functions()) {
        generate_proxy_interface(class_);
        header_ << h_off_ << "using " << class_->name()
                << "_prx = ::std::shared_ptr< " << class_->name() << "_proxy >;";
        header_ << "\n";
    }
}

void
generator::finish_compilation_unit(ast::compilation_unit const& u)
{
    qname const wire_enc_detail{ "::wire::encoding::detail" };
    ast::entity_const_set exceptions;
    u.collect_elements(
        exceptions,
        [](ast::entity_const_ptr e)
        {
            return (bool)ast::dynamic_entity_cast< ast::exception >(e).get();
        });
    if (!exceptions.empty()) {
        adjust_scope(wire_enc_detail.search());
        for (auto ex : exceptions) {
            header_ << h_off_ << "template <>"
                    << h_off_ << "struct is_user_exception< " << rel_name(ex->get_qualified_name())
                        << " > : ::std::true_type {};";
        }
    }
    ast::entity_const_set interfaces;
    u.collect_elements(
        interfaces,
        [](ast::entity_const_ptr e)
        {
            auto iface = ast::dynamic_entity_cast< ast::interface >(e);
            if (iface) {
                auto cls = ast::dynamic_entity_cast<ast::class_>(iface);
                return !cls || cls->has_functions();
            }
            return false;
        });
    if (!interfaces.empty()) {
        adjust_scope(wire_enc_detail.search());
        for (auto iface : interfaces) {
            header_ << h_off_ << "template <>"
                    << h_off_ << "struct is_proxy< " << rel_name(iface->get_qualified_name()) << "_proxy"
                        << " >: ::std::true_type {};";
        }
    }
}

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */
