#pragma once

#include <string>
#include <fc/time.hpp>

namespace steemit {
    namespace application {

        using namespace steemit::chain;
        using namespace steemit::protocol;


        struct scheduled_hardfork {
            hardfork_version hf_version;
            fc::time_point_sec live_time;
        };

        struct withdraw_route {
            std::string from_account;
            std::string to_account;
            uint16_t percent;
            bool auto_vest;
        };

        enum withdraw_route_type {
            incoming,
            outgoing,
            all
        };


    }
}

FC_REFLECT(steemit::application::scheduled_hardfork, (hf_version)(live_time));
FC_REFLECT(steemit::application::withdraw_route, (from_account)(to_account)(percent)(auto_vest));

FC_REFLECT_ENUM(steemit::application::withdraw_route_type, (incoming)(outgoing)(all));