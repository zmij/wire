/*
 * plugin.cpp
 *
 *  Created on: Dec 4, 2017
 *      Author: zmij
 */

#include <wire/util/plugin.hpp>
#include <stdexcept>
#include <sstream>
#include <dlfcn.h>


namespace wire {
namespace util {

plugin::plugin(::std::string const& name)
    : name_{name}, handle_{nullptr}
{
    handle_ = ::dlopen(name.c_str(), RTLD_LAZY);
    if (!handle_) {
        ::std::ostringstream os;
        os << "Failed to open library " << name_ << ": " << ::dlerror();
        throw ::std::runtime_error{ os.str() };
    }
}

plugin::~plugin()
{
    if (handle_) {
        ::dlclose(handle_);
    }
}

void*
plugin::get_symbol(::std::string const& name)
{
    if (!handle_)
        throw ::std::runtime_error{"Plugin library " + name_ + " is not loaded"};
    auto f = symbols_.find(name);
    if (f == symbols_.end()) {
        // Reset errors
        ::dlerror();
        void* sym = ::dlsym(handle_, name.c_str());
        char const* err = ::dlerror();
        if (err) {
            ::std::ostringstream os;
            os << "Failed to load symbol: " << err;
            throw ::std::runtime_error{ os.str() };
        }
        auto res = symbols_.emplace(name, sym);
        f = res.first;
    }
    return f->second;
}

//----------------------------------------------------------------------------
plugin_manager&
plugin_manager::instance()
{
    static plugin_manager _instance;
    return _instance;
}

plugin&
plugin_manager::get_plugin(::std::string const& name)
{
    auto f = plugins_.find(name);
    if (f == plugins_.end()) {
        auto res = plugins_.emplace(name, name);
        f = res.first;
    }
    return f->second;
}

} /* namespace util */
} /* namespace wire */
