#include <steemit/blockchain_statistics/statistics_sender.hpp>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>

#include <fc/exception/exception.hpp>
#include <boost/exception/all.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>

#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>

namespace steemit {
namespace blockchain_statistics {

statistics_sender::statistics_sender(uint32_t default_port) : default_port_(default_port) {
}

statistics_sender::~statistics_sender( ) {
    QUEUE_ENABLED = false;
    queue_.synchronize();

    if ( !recipient_endpoint_set_.empty() ) {
        sender_thread_.join();
    }
}

bool statistics_sender::can_start() {
    return !recipient_endpoint_set_.empty();
}

void statistics_sender::push(const std::string & str) {
    queue_.push(str);
}

void statistics_sender::start_sending() {
    QUEUE_ENABLED = true;

    auto run_broadcast_loop = [&]() {
        boost::asio::io_service io_service;

        boost::asio::ip::udp::socket socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
        socket.set_option(boost::asio::socket_base::broadcast(true));

        while ( QUEUE_ENABLED.load() ) {
            auto str_to_send = queue_.wait_pop();
            for (auto endpoint : recipient_endpoint_set_) {
                socket.send_to(boost::asio::buffer(str_to_send), endpoint);
            }        
        }
    };

    try {   
        std::thread tmp_thread(run_broadcast_loop);
        sender_thread_ = std::move(tmp_thread);
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }
}

void statistics_sender::add_address(const std::string & address) {
    // Parsing "IP:PORT". If there is no port, then use Default one from configs.
    try
    {
        boost::asio::ip::udp::endpoint ep;
        boost::asio::ip::address ip;
        uint16_t port;        
        boost::system::error_code ec;

        auto pos = address.find(':');

        if (pos != std::string::npos) {
            ip = boost::asio::ip::address::from_string( address.substr( 0, pos ) , ec);
            port = boost::lexical_cast<uint16_t>( address.substr( pos + 1, address.size() ) );
        }
        else {
            ip = boost::asio::ip::address::from_string( address , ec);
            port = default_port_;
        }
        
        if (ip.is_unspecified()) {
            // TODO something with exceptions and logs!
            ep = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port);
            recipient_endpoint_set_.insert(ep);
        }
        else {
            ep = boost::asio::ip::udp::endpoint(ip, port);
            recipient_endpoint_set_.insert(ep);
        }
    }
    FC_CAPTURE_AND_LOG(())
}

std::vector<std::string> statistics_sender::get_endpoint_string_vector() {
    std::vector<std::string> ep_vec;
    for (auto x : recipient_endpoint_set_) {
        ep_vec.push_back( x.address().to_string() + ":" + std::to_string(x.port()));
    }
    return ep_vec;
}

} } // steemit::blockchain_statistics