#pragma once

#include <steemit/blockchain_statistics/concurrent_queue.hpp>
#include <thread>
#include <vector>
#include <atomic>

#include <fc/api.hpp>
#include <fc/uint128.hpp>
#include <boost/asio/ip/udp.hpp>

namespace steemit {
namespace blockchain_statistics {

class statistics_sender final {
public:
    statistics_sender() = default;
    statistics_sender(uint32_t default_port);

    ~statistics_sender();

    bool can_start();

    // endless cycle for wait-pop from queue 
    void start_sending();

    // pushes string to the queue
    void push(const std::string & str);
    
    // adds address to recipient_endpoint_set_.
    void add_address(const std::string & address);

    /// returns statistics recievers endpoints
    std::vector<std::string> get_endpoint_string_vector();

private:
    concurrent_queue<std::string> queue_;
    // Stat sender will send data to all endpoints from recipient_endpoint_set_
    std::set<boost::asio::ip::udp::endpoint> recipient_endpoint_set_;
    // DefaultPort for asio broadcasting 
    uint32_t default_port_;
    // Flag which indicates is statistics_sender is enabled or not
    std::atomic_bool QUEUE_ENABLED;

    std::thread sender_thread_;
};
}} // steemit::blockchain_statistics
