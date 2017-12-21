/*
 * cpp_source_stream.cpp
 *
 *  Created on: May 13, 2016
 *      Author: zmij
 */

#include <wire/wire2cpp/cpp_source_stream.hpp>
#include <wire/idl/syntax_error.hpp>
#include <wire/idl/generator.hpp>
#include <wire/wire2cpp/keywords.hpp>

#include <sstream>
#include <iomanip>

namespace wire {
namespace idl {
namespace cpp {

namespace fs = ::boost::filesystem;

namespace {

::std::string const autogenerated =
R"~(/*
 * THIS FILE IS AUTOGENERATED BY wire2cpp PROGRAM
 * Any manual modifications can be lost
 */
)~";

::std::string
create_header_guard(fs::path const& inc_dir, fs::path const& fname)
{
    ::std::ostringstream os;
    os << "_";
    for (auto const& p : inc_dir) {
        os << p.string() << "_";
    }
    os << fname.stem().string() << "_HPP_";

    auto hg = os.str();
    ::std::transform(hg.begin(), hg.end(),
            hg.begin(), toupper);

    return hg;
}

void
write_name(::std::ostream& os, ast::qname const& qn, ast::qname const& current_scope, bool& write_abs)
{
    ::std::ostream::sentry s{os};
    if (s) {
        if (write_abs) {
            os << cpp_name(qn);
            write_abs = false;
        } else {
            os << cpp_name(qn.in_scope(current_scope));
        }
    }
}

void
write_offset(::std::ostream& os, int space_number)
{
    if (space_number > 0)
        os << ::std::setw( space_number ) << ::std::setfill(' ') << " ";
}

}  /* namespace  */

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

source_stream::source_stream(path const& origin_path, path const& header_dir,
        string_list const& include_dirs)
    : origin_path_{origin_path},
      header_dir_{header_dir},
      is_header_{false},
      current_namespace_{true},
      scope_{false},
      current_offset_{0},
      tab_width_{4},
      abs_name_{false}
{
    for (auto const& inc : include_dirs) {
        path inc_path{inc};
        if (inc_path.is_relative() && fs::exists(inc_path)) {
            inc_path = fs::canonical(inc_path);
        }
        if (fs::exists(inc_path)) {
            include_dirs_.push_back(inc_path.string());
        }
    }
}

source_stream::~source_stream()
{
    adjust_namespace(ast::qname{});
    if (is_header_) {
        stream_ << "#endif          /* " << header_guard_ << " */\n";
    }
}

void
source_stream::open(path const& p)
{
    stream_.open(p.string());
    stream_ << autogenerated;
    if (p.extension() == ".hpp") {
        is_header_ = true;
        header_guard_ = create_header_guard(header_dir_, p.filename());

        stream_ << "#ifndef " << header_guard_ << "\n"
                << "#define " << header_guard_ << "\n\n";
    } else {
        // include header
        path head = p.stem();
        head += ::std::string{".hpp"};
        include(head);
    }
}

source_stream&
source_stream::include(::std::string const& include_str)
{
    stream_ << "#include " << include_str << "\n";
    return *this;
}

source_stream&
source_stream::include(path inc_file)
{
    if (inc_file.extension() == ".wire") {
        inc_file.replace_extension(".hpp");
    }
    if (inc_file.is_relative() && !inc_file.has_parent_path()) {
        if (!header_dir_.empty()) {
            include(header_dir_ / inc_file.filename());
        } else {
            stream_ << "#include \"" << inc_file.string() << "\"\n";
        }
    } else if (!inc_file.is_relative()) {
        // Absolute path, as preprocessor feeds
        auto file_path = inc_file.string();
        auto orig = origin_path_.string();
        if (!orig.empty() && file_path.find(orig) == 0) {
            // Same directory or below
            path p { file_path.substr(orig.size() + 1) };
            if (!header_dir_.empty()) {
                include(header_dir_ / p);
            } else{
                include(p);
            }
        } else {
            // Include from one of include directories
            for (auto const& inc_dir : include_dirs_) {
                if (file_path.find(inc_dir) == 0) {
                    path p { file_path.substr(inc_dir.size() + 1) };
                    include(p);
                    break;
                }
            }
        }
    } else {
        // Relative path with parent path
        stream_ << "#include <" << inc_file.string() << ">\n";
    }
    return *this;
}


void
source_stream::start_namespace(::std::string const& name)
{
    if (!scope_.empty()) {
        throw ::std::runtime_error{"Cannot start a namespace in a non-namespace scope"};
    }
    stream_ << "namespace " << cpp_name(name) << "{\n";
    current_namespace_.components.push_back(name);
}

void
source_stream::push_scope(::std::string const& name)
{
    scope_.components.push_back(name);
    ++current_offset_;
}

void
source_stream::pop_scope()
{
    if (!scope_.empty()) {
        scope_.components.pop_back();
        --current_offset_;
        if (scope_.empty()) {
            // call functions that want to be at namespace level
            for (auto const& cb: at_namespace_scope_) {
                cb(*this);
            }
            at_namespace_scope_.clear();
        }
    } else {
        if (current_namespace_.empty()) {
            throw ::std::runtime_error{"Cannot pop a scop at global scope"};
        }
        stream_ << "} /* namespace " << current_namespace_.name() << " */\n";
        current_namespace_.components.pop_back();
    }
}

void
source_stream::pop_to_namespace()
{
    while (!scope_.empty()) {
        scope_.components.pop_back();
        --current_offset_;
    }
    if (scope_.empty()) {
        // call functions that want to be at namespace level
        for (auto const& cb: at_namespace_scope_) {
            cb(*this);
        }
        at_namespace_scope_.clear();
    }
}

void
source_stream::at_namespace_scope(callback cb)
{
    if (scope_.empty()) {
        cb(*this);
    } else {
        at_namespace_scope_.push_back(cb);
    }
}

void
source_stream::adjust_namespace(ast::qname const& target_scope)
{
    ast::qname_search target = target_scope.search();
    ast::qname_search current = current_namespace_.search();

    auto c = current.begin;
    auto t = target.begin;

    for (; c != current.end && t != target.end && *c == *t; ++c, ++t);
    auto erase_start = c;
    if (erase_start != current_namespace_.end()) {
        stream_ << "\n";
    }
    for (; c != current.end; ++c) pop_scope();
    if (erase_start != current_namespace_.end()) {
        stream_ << "\n";
    }

    bool space = t != target.end;
    for (; t != target.end; ++t) {
        start_namespace(*t);
    }
    if (space) {
        stream_ << "\n";
        current_offset_ = 0;
    }
}

void
source_stream::adjust_namespace(ast::entity_ptr ent)
{
    auto qn = ent->get_namespace()->get_qualified_name();
    adjust_namespace(qn);
}

ast::qname
source_stream::current_scope() const
{
    return current_namespace_ + scope_;
}

void
source_stream::write(ast::qname const& qn)
{
    write_name(stream_, qn, current_namespace_ + scope_, abs_name_);
}

void
source_stream::write_offset(int temp)
{
    stream_ << "\n";
    cpp::write_offset(stream_, (current_offset_ + temp) * tab_width_);
}

void
source_stream::modify_offset(int delta)
{
    current_offset_ += delta;
    if (current_offset_ < 0)
        current_offset_ = 0;
}

void
source_stream::set_offset(int offset)
{
    current_offset_ = offset;
    if (current_offset_ < 0)
        current_offset_ = 0;
}

bool
is_primitive(::std::string const& name)
{
    return name != ast::STRING && name != ast::UUID;
}

bool
is_primitive(ast::type_const_ptr t)
{
    if (auto pt = ast::dynamic_entity_cast< ast::parametrized_type >(t)) {
        return false;
    } else if (ast::type::is_built_in(t->get_qualified_name())) {
        return is_primitive(t->name());
    } else if (auto en = ast::dynamic_type_cast<ast::enumeration>(t)) {
        return true;
    } else if (auto alias = ast::dynamic_entity_cast< ast::type_alias >(t)) {
        return is_primitive(alias->alias());
    }
    return false;
}

template < typename StreamType >
StreamType&
write (StreamType& os, mapped_type const& val)
{
    if (auto pt = ast::dynamic_entity_cast< ast::parametrized_type >(val.type)) {
        auto tmpl_name = wire_to_cpp(pt->name()).type_name;
        if (pt->name() == ast::ARRAY ||
                pt->name() == ast::SEQUENCE ||
                pt->name() == ast::DICTONARY) {
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
                    os << mapped_type{ ::boost::get< ast::type_ptr >(*p) };
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
        } else if (val.is_dispatch_arg) {
            os << "&&";
        }
    } else {
        if (ast::type::is_built_in(val.type->get_qualified_name())) {
            os << wire_to_cpp( val.type->name() ).type_name;
            if (val.is_arg && !is_primitive(val.type->name())) {
                os << " const&";
            } else if (val.is_dispatch_arg && !is_primitive(val.type->name())) {
                os << "&&";
            }
        } else if (auto ref = ast::dynamic_entity_cast< ast::reference >(val.type)) {
            os << qname(val.type) << "_prx";
        } else if (auto cl = ast::dynamic_type_cast< ast::class_ >(val.type)) {
            os << qname(val.type) << "_ptr";
        } else if (auto iface = ast::dynamic_type_cast< ast::interface >(val.type)) {
            os << qname(val.type) << "_ptr";
        } else {
            auto scope = val.type->get_global()->find_scope(os.current_scope()).first;
            if (scope) {
                auto type_name = val.type->get_qualified_name();
                os << cpp_safe_qname_search{ type_name.in_scope(scope) };
            } else {
                // The only type defining scope that is absent from AST
                // is proxy type.
                // TODO Calculate type name relative to proxy's scope
                os << qname(val.type);
            }
            if (val.is_arg && !is_primitive(val.type)) {
                os << " const&";
            } else if (val.is_dispatch_arg && !is_primitive(val.type)) {
                os << "&&";
            }
        }
    }
    return os;
}

template < typename StreamType >
StreamType&
write(StreamType& os, invoke_param const& v)
{
    bool need_move = false;
    if (auto pt = ast::dynamic_entity_cast< ast::parametrized_type >(v.param.first)) {
        need_move = true;
    } else if (ast::type::is_built_in(v.param.first->get_qualified_name())) {
        need_move = !is_primitive(v.param.first->name());
    } else if (auto ref = ast::dynamic_entity_cast< ast::reference >(v.param.first)) {
    } else if (auto cl = ast::dynamic_type_cast< ast::class_ >(v.param.first)) {
    } else if (auto iface = ast::dynamic_type_cast< ast::interface >(v.param.first)) {
    } else {
        need_move = !is_primitive(v.param.first);
    }
    if (need_move) {
        os << "::std::move(" << v.param.second << ")";
    } else {
        os << v.param.second;
    }
    return os;
}

source_stream&
operator << (source_stream& os, mapped_type const& v)
{
    return write(os, v);
}

source_stream&
operator << (source_stream& os, invoke_param const& v)
{
    return write(os, v);
}

source_stream&
operator << (source_stream& os, grammar::data_initializer const& init)
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
                    os << *(*p);
                }
                os << "}";
            } else {
                os << "{" << mod(+1);
                for (auto p = list.begin(); p != list.end(); ++p) {
                    if (p != list.begin())
                        os << "," << off;
                    os << *(*p);
                }
                os << mod(-1) << "}";
            }
            break;
        }
    }
    return os;
}

//----------------------------------------------------------------------------
//      code_snippet
//----------------------------------------------------------------------------
void
code_snippet::write(ast::qname const& qn)
{
    write_name(os_, qn, current_scope_, abs_name_);
}

void
code_snippet::write_offset(int temp)
{
    lines_.emplace_back(current_offset_ + current_mod_, os_.str());
    os_.str(::std::string{});

    current_mod_ = temp;
}

void
code_snippet::modify_offset(int delta)
{
    current_offset_ += delta;
    if (current_offset_ < 0)
        current_offset_ = 0;
}

::std::string
code_snippet::str(int tab_width) const
{
    ::std::ostringstream os;
    if (!lines_.empty()) {
        auto l = lines_.begin();
        os << l->second;
        for (++l; l != lines_.end(); ++l) {
            cpp::write_offset(os, l->first * tab_width);
            os << l->second;
        }
    }
    if (!os_.str().empty()) {
        if (!lines_.empty())
            cpp::write_offset(os, (current_offset_ + current_mod_) * tab_width);
        os << os_.str();
    }
    return os.str();
}

code_snippet&
operator << (code_snippet& os, mapped_type const& v)
{
    return write(os, v);
}

code_snippet&
operator << (code_snippet& os, invoke_param const& v)
{
    return write(os, v);
}

source_stream&
operator << (source_stream& os, code_snippet const& cs)
{
    if (!cs.lines_.empty()) {
        auto l = cs.lines_.begin();
        os << l->second;
        for (++l; l != cs.lines_.end(); ++l) {
            os.write_offset(l->first);
            os << l->second;
        }
    }
    if (!cs.os_.str().empty()) {
        if (!cs.lines_.empty())
            os.write_offset(cs.current_offset_ + cs.current_mod_);
        os << cs.os_.str();
    }
    return os;
}

code_snippet&
operator << (code_snippet& os, code_snippet const& cs)
{
    if (!cs.lines_.empty()) {
        auto l = cs.lines_.begin();
        os << l->second;
        for (++l; l != cs.lines_.end(); ++l) {
            os.write_offset(l->first);
            os << l->second;
        }
    }
    if (!cs.os_.str().empty()) {
        if (!cs.lines_.empty())
            os.write_offset(cs.current_offset_ + cs.current_mod_);
        os << cs.os_.str();
    }
    return os;
}

//----------------------------------------------------------------------------
//      cpp safe names
//----------------------------------------------------------------------------
::std::ostream&
operator << (::std::ostream& os, cpp_safe_name const& val)
{
    ::std::ostream::sentry s (os);
    if (s) {
        os << val.name;
        if (keywords.count(val.name))
            os << "_";
    }
    return os;
}

::std::ostream&
operator << (::std::ostream& os, cpp_safe_qname const& val)
{
    ::std::ostream::sentry s (os);
    if (s) {
        os << cpp_name(val.qn.search());
    }
    return os;
}

::std::ostream&
operator << (::std::ostream& os, cpp_safe_qname_search const& val)
{
    ::std::ostream::sentry s (os);
    if (s) {
        if (val.qs.fully)
            os << "::";
        for (auto n = val.qs.begin; n != val.qs.end; ++n) {
            if (n != val.qs.begin)
                os << "::";
            os << cpp_name(*n);
        }
    }
    return os;
}


}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */
