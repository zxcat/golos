#pragma once

#include <golos/plugins/json_rpc/utility.hpp>

#include <golos/protocol/types.hpp>

#include <fc/optional.hpp>

namespace golos {
    namespace plugins {
        namespace chain {
            struct push_block_args {
                golos::chain::signed_block block;
                bool currently_syncing = false;
            };


            struct push_block_return {
                bool success;
                optional <string> error;
            };

            typedef golos::chain::signed_transaction push_transaction_args;

            struct push_transaction_return {
                bool success;
                optional <string> error;
            };


            class api {
            public:
                api();

                ~api();

                DECLARE_API((push_block)(push_transaction))

            private:
                class api_impl;

                std::unique_ptr<api_impl> my;
            };

        }
    }
} // golos::plugins::chain

FC_REFLECT((golos::plugins::chain::push_block_args), (block)(currently_syncing))
FC_REFLECT((golos::plugins::chain::push_block_return), (success)(error))
FC_REFLECT((golos::plugins::chain::push_transaction_return), (success)(error))
