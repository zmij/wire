/*
 * plugin.hpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#ifndef WIRE_UTIL_PLUGIN_HPP_
#define WIRE_UTIL_PLUGIN_HPP_

#include <string>
#include <map>

namespace wire {
namespace util {

class plugin {
public:
    plugin(::std::string const& name);

    plugin(plugin const&) = delete;
    plugin(plugin&& rhs)
        : name_{::std::move(rhs.name_)},
          handle_{rhs.handle_},
          symbols_{ ::std::move(rhs.symbols_) }
    {
        rhs.handle_ = nullptr;
    }
    ~plugin();
    plugin&
    operator =(plugin const&) = delete;
    plugin&
    operator =(plugin&& rhs)
    {
        swap(rhs);
        return *this;
    }

    void
    swap(plugin& rhs)
    {
        using ::std::swap;
        swap(name_,     rhs.name_);
        swap(handle_,   rhs.handle_);
        swap(symbols_,  rhs.symbols_);
    }

    template < typename Ret, typename ... Args >
    Ret
    call(::std::string const& entry, Args ... args)
    {
        using func_type = Ret(Args...);
        func_type* f = nullptr;
        *reinterpret_cast<void**>(&f) = get_symbol(entry);
        return f(args...);
    }
private:
    void*
    get_symbol(::std::string const& name);
private:
    using symbols       = ::std::map< ::std::string, void* >;

    ::std::string   name_;
    void*           handle_     = nullptr;
    symbols         symbols_;
};

class plugin_manager {
public:
    static plugin_manager&
    instance();

    plugin&
    get_plugin(::std::string const& name);
private:
    plugin_manager() {}
    plugin_manager(plugin_manager const&) = delete;
    plugin_manager(plugin_manager&&) = delete;
    plugin_manager&
    operator =(plugin_manager const&) = delete;
    plugin_manager&
    operator =(plugin_manager&&) = delete;

private:
    using plugins = ::std::map< ::std::string, plugin >;
    plugins plugins_;
};

} /* namespace util */
} /* namespace wire */




#endif /* WIRE_UTIL_PLUGIN_HPP_ */
