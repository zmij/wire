/*
 * ping_pong_sparring.cpp
 *
 *  Created on: May 8, 2016
 *      Author: zmij
 */

#include <iostream>
#include <fstream>
#include "ping_pong_impl.hpp"

#include <wire/core/connector.hpp>
#include <wire/core/adapter.hpp>
#include <wire/core/proxy.hpp>

#include <boost/program_options.hpp>

class stream_redirect {
public:
    stream_redirect(::std::ostream& stream,
            ::std::string const& file_name,
            ::std::ios_base::openmode open_mode
                 = ::std::ios_base::out | ::std::ios_base::app) :
        stream_(stream), old_(stream.rdbuf())
    {
        file_.open(file_name.c_str(), open_mode);
        if (!file_.good()) {
            std::ostringstream msg;
            msg << "Failed to open file " << file_name
                    << ": " << strerror(errno) << "\n";
            throw std::runtime_error(msg.str());
        }
        stream_.rdbuf(file_.rdbuf());
    }
    ~stream_redirect()
    {
        stream_.flush();
        stream_.rdbuf(old_);
    }
private:
    std::ostream& stream_;
    std::streambuf* old_;
    std::ofstream file_;
};

namespace {

::std::shared_ptr<stream_redirect> redirect_cerr;

}  /* namespace  */

int
main(int argc, char* argv[])
try {
    using namespace ::wire::core;
    namespace po = ::boost::program_options;
    ::std::uint16_t port_no{0};
    po::options_description desc("Test options");
    ::std::string log_file;

    desc.add_options()
        ("help,h", "show options description")
        ("port,p",
            po::value< ::std::uint16_t >(&port_no),
            "Port to bind to")
        ("log", po::value< ::std::string >(&log_file))
    ;
    po::parsed_options parsed_cmd =
            po::command_line_parser(argc, argv).options(desc).
                        allow_unregistered().run();

    po::variables_map vm;
    po::store(parsed_cmd, vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    auto unrecognized_cmd = po::collect_unrecognized(parsed_cmd.options, po::exclude_positional);
    po::notify(vm);

    if (!log_file.empty()) {
        redirect_cerr =
            std::make_shared< stream_redirect >( std::cerr,
                    log_file );
    }

    ::wire::asio_config::io_service_ptr io_service =
            ::std::make_shared< ::wire::asio_config::io_service >();

    asio_ns::signal_set signals(*io_service);
    signals.add(SIGINT);
    signals.add(SIGTERM);
    signals.add(SIGQUIT);
    signals.async_wait(
    [io_service](::wire::asio_config::error_code const&, int signo){
        ::std::cerr << "Signal " << signo << "\n";
        io_service->stop();
    });

    connector_ptr conn = connector::create_connector(io_service, unrecognized_cmd);
    adapter_ptr adptr = conn->create_adapter(
            "ping_pong", { ::wire::core::endpoint::tcp("127.0.0.1", port_no) });

    adptr->activate();
    auto pp_server = ::std::make_shared< wire::test::ping_pong_server >([io_service](){ io_service->stop(); });
    auto prx = adptr->add_object({"ping_pong"}, pp_server);
    adptr->add_default_servant(pp_server);
    ::std::cout << *prx << ::std::endl;
    if (prx->wire_get_reference()->is_local()) {
        ::std::cerr << "Reference classified to be local\n";
    } else {
        ::std::cerr << "Reference classified to be remote\n";
    }

    io_service->run();
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
