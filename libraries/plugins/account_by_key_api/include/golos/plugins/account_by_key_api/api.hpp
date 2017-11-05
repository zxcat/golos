#pragma once

#include <golos/plugins/json_rpc/utility.hpp>

#include <golos/protocol/types.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

namespace golos {
    namespace plugins {
        namespace account_by_key {

            namespace detail {
                class api_impl;
            }

            struct get_key_references_args {
                std::vector<golos::protocol::public_key_type> keys;
            };

            struct get_key_references_return {
                std::vector<std::vector<golos::protocol::account_name_type> > accounts;
            };

            class api {
            public:
                api();

                ~api();

                DECLARE_API((get_key_references))

            private:
                std::unique_ptr<detail::api_impl> my;
            };
        }
    }
} // golos::plugins::account_by_key

FC_REFLECT((golos::plugins::account_by_key::get_key_references_args), (keys))

FC_REFLECT((golos::plugins::account_by_key::get_key_references_return), (accounts))