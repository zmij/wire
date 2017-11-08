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

#include <wire/core/detail/configuration_options.hpp>

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
    transport_type transport;
    ::std::string log_file;
    ::std::size_t   thread_count{1};
    po::options_description desc("Test options");

    desc.add_options()
        ("help,h", "show options description")
        ("transport,t",
            po::value<transport_type>(&transport)->default_value(transport_type::tcp),
            "Start server for transport [tcp, ssl, udp, socket]")
        ("port,p",
            po::value< ::std::uint16_t >(&port_no),
            "Port to bind to")
        ("threads", po::value<::std::size_t>(&thread_count)->default_value(1),
            "Number of threads to run")
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
        ::std::cerr << ::getpid() << " Signal " << signo << "\n";
        io_service->stop();
    });

    connector_ptr conn = connector::create_connector(io_service, unrecognized_cmd);
    endpoint ep;
    switch (transport) {
        case transport_type::tcp:
            ep = endpoint::tcp("127.0.0.1", port_no);
            break;
        case transport_type::ssl:
            ep = endpoint::ssl("127.0.0.1", port_no);
            break;
        case transport_type::socket:
            ep = endpoint::socket("/tmp/.ping_pong." + ::std::to_string(::getpid()));
            break;
        default:
            throw ::std::runtime_error{ "Protocol " + to_string(transport) + " is not implemented for sparring" };
    }
    adapter_ptr adptr = conn->create_adapter( "ping_pong", { ep });

    adptr->activate();
    auto pp_server = ::std::make_shared< wire::test::ping_pong_server >([io_service](){ io_service->stop(); });
    auto prx = adptr->add_object({"ping_pong"}, pp_server);
    adptr->add_default_servant(pp_server);
    ::std::cout << *prx << ::std::endl;

    if (thread_count > 1) {
        if (thread_count > ::std::thread::hardware_concurrency())
            thread_count = ::std::thread::hardware_concurrency();
        ::std::cerr << ::getpid() << " running in " << thread_count << " threads\n";
        ::std::vector< ::std::thread > threads;
        for (auto i = 0U; i < thread_count; ++i) {
            threads.emplace_back(::std::thread{ [&](){
                io_service->run();
            }});
        }
        for (auto& t : threads) {
            t.join();
        }
    } else {
        io_service->run();
    }
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
