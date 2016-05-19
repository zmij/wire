/*
 * connector_admin.cpp
 *
 *  Created on: May 19, 2016
 *      Author: zmij
 */

#include <iostream>
#include <iomanip>

#include <boost/program_options.hpp>
#include <wire/util/console.hpp>
#include <wire/core/connector_admin.hpp>
#include <wire/core/connector.hpp>
#include <wire/core/reference.hpp>
#include <wire/errors/exceptions.hpp>

using ::wire::util::console;
namespace core = ::wire::core;

struct options {
    core::reference_data    admin_ref;
};

struct object_prompt {
    core::object_prx        prx;

    ::std::string
    operator()() const
    {
        ::std::ostringstream os;
        os << *prx << " # ";
        return os.str();
    }
};

struct adapter_status_cmd {
    core::adapter_admin_prx adapter_;

    console::ret_code_type
    operator()(console::args_type const&)
    {
        try {
            auto status = adapter_->get_state();
            if (status == core::adapter_state::active) {
                ::std::cout << "active\n";
            } else {
                ::std::cout << "inactive\n";
            }
        } catch (core::no_adapter const&) {
            ::std::cout << "No such adapter\n";
        }
        return console::ok;
    }
};

struct adapter_activate_cmd {
    core::adapter_admin_prx adapter_;

    console::ret_code_type
    operator()(console::args_type const&)
    {
        try {
            adapter_->activate();
        } catch (core::no_adapter const&) {
            ::std::cout << "No such adapter\n";
        } catch (core::adapter_active const&) {
            ::std::cout << "Adapter already active\n";
        }
        return console::ok;
    }
};

struct adapter_deactivate_cmd {
    core::adapter_admin_prx adapter_;

    console::ret_code_type
    operator()(console::args_type const&)
    {
        try {
            adapter_->deactivate();
        } catch (core::no_adapter const&) {
            ::std::cout << "No such adapter\n";
        } catch (core::adapter_inactive const&) {
            ::std::cout << "Adapter already inactive\n";
        }
        return console::ok;
    }
};

struct adapter_console {
    core::adapter_admin_prx adapter_;
    console                 cons_;

    adapter_console(core::adapter_admin_prx adapter)
        : adapter_{ adapter }, cons_{ object_prompt{adapter}, false }
    {
        cons_.add_commands()
                ("q",           cons_.quit_command("Cancel"))
                ("help",        cons_.help_command())
                ("status",      adapter_status_cmd{ adapter_ })
                ("activate",    adapter_activate_cmd{ adapter_ })
                ("deactivate",  adapter_deactivate_cmd{ adapter_ })
        ;
    }

    console::ret_code_type
    operator()()
    {
        auto ret = cons_.read_line();
        while (ret != console::quit && ret != console::eof)
            ret = cons_.read_line();
        return ret;
    }
};

struct select_adapter {
    core::connector_admin_prx   admin_;
    console                     cons_;
    core::adapter_admin_prx     adapter_;

    select_adapter(core::connector_admin_prx admin)
        : admin_{admin}, cons_{[](){ return "Select adapter > "; }, false}
    {
        cons_.add_command("q", cons_.quit_command("Cancel"));
        auto ids = admin_->get_adapter_ids();
        if (!ids.empty()) {
            ::std::cout << "Select adapters:\n";
            auto no = 1;
            for (auto const& id : ids) {
                ::std::cout << ::std::setw(4) << ::std::left << no
                        << id << "\n";
                cons_.add_command( ::std::to_string(no),
                    [this, id](console::args_type const&)
                    {
                        adapter_ = admin_->get_adapter_admin(id);
                        return console::quit;
                    }
                );
                ++no;
            }
        } else {
            ::std::cout << "No adapters available\n";
        }
    }
    console::ret_code_type
    operator()()
    {
        if (cons_.get_commands().size() > 1) {
            auto ret = cons_.read_line();
            while (ret != console::quit && ret != console::eof)
                ret = cons_.read_line();
            return ret;
        }
        return console::quit;
    }
};

struct list_adapters_cmd {
    ::wire::core::connector_admin_prx admin_;
    console::ret_code_type
    operator()(console::args_type const& args) const
    {
        auto ids = admin_->get_adapter_ids();
        if (!ids.empty()) {
            ::std::cout << "Available adapters:\n";
            for (auto const& id : ids) {
                ::std::cout << "\t" << id << "\n";
            }
        } else {
            ::std::cout << "No adapters available\n";
        }
        return console::ok;
    }
};

struct use_adapter_cmd {
    core::connector_admin_prx       admin_;
    core::adapter_admin_prx mutable adapter_;

    console::ret_code_type
    operator()(console::args_type const& args) const
    {
        if (args.size() > 1) {
            ::std::istringstream is{args[1]};
            core::identity id;
            if (is >> id) {
                try {
                    adapter_ = admin_->get_adapter_admin(id);
                } catch (core::no_adapter const&) {
                    ::std::cout << "Adapter " << args[1] << " is not known\n";
                    return console::error;
                }
            } else {
                ::std::cout << "String " << args[1] << " cannot be transformed to an identity\n";
                return console::error;
            }
        } else {
            select_adapter sel{admin_};
            auto ret = sel();
            if (ret != console::quit)
                return ret;
            adapter_ = sel.adapter_;
        }
        if (adapter_) {
            ::std::cout << "Using adapter " << *adapter_ << "\n";
            adapter_console cons{adapter_};
            auto ret = cons();
            if (ret == console::eof)
                return ret;
        }
        return console::ok;
    }
};

int
main(int argc, char* argv[])
try {
    namespace po = ::boost::program_options;

    options opts;

    po::options_description desc("Program options");

    desc.add_options()
        ("help,h", "show options description")
        ("proxy,p", po::value< core::reference_data >(&opts.admin_ref)->required(),
                "Proxy of connector admin object")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    po::notify(vm);

    ::wire::asio_config::io_service_ptr io_service =
            ::std::make_shared<::wire::asio_config::io_service>();
    core::connector_ptr conn = core::connector::create_connector(io_service);
    core::object_prx obj = conn->make_proxy(opts.admin_ref);
    core::connector_admin_prx admin_prx =
            core::checked_cast< core::connector_admin_proxy >(obj);

    if (!admin_prx)
        throw ::wire::errors::runtime_error{"Failed to connect to admin object at ",
            *admin_prx};

    console cnctr_cons{object_prompt{admin_prx}};

    cnctr_cons.add_commands()
        ("ls", console::command{ list_adapters_cmd{admin_prx}, [](){ return  "List adapters"; }})
        ("use", console::command{ use_adapter_cmd{admin_prx}, [](){ return "Use adapter admin";}});

    auto ret = cnctr_cons.read_line();
    while (ret != console::quit && ret != console::eof) {
        ret = cnctr_cons.read_line();
    }
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
