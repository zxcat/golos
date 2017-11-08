#pragma once

#include <golos/plugins/json_rpc/utility.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

#include <golos/protocol/types.hpp>

#include <fc/api.hpp>
#include <fc/crypto/sha256.hpp>

#include <string>

namespace golos {
namespace plugins {
namespace auth_util_api {
    using golos::plugins::json_rpc::msg_pack;

    struct check_authority_signature_a {
        std::string account_name;
        std::string level;
        fc::sha256 dig;
        std::vector<protocol::signature_type> sigs;
    };

    struct check_authority_signature_r {
        std::vector<protocol::public_key_type> keys;
    };

    DEFINE_API_ARGS ( check_authority_signature, msg_pack, check_authority_signature_r)

    class api final {
    public:
        api();

        ~api() = default;

        DECLARE_API((check_authority_signature))

    protected:
        class api_impl;

        std::shared_ptr<api_impl> my;
    };
}
}
} // golos::plugins::api

FC_REFLECT((golos::plugins::auth_util_api::check_authority_signature_a), (account_name)(level)(dig)(sigs))

FC_REFLECT((golos::plugins::auth_util_api::check_authority_signature_r), (keys))