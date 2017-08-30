#pragma once

#include <steemit/protocol/base.hpp>

#include <steemit/plugins/follow/follow_plugin.hpp>
#include <steemit/api_object/helper.hpp>

namespace steemit {
    namespace plugins {
        namespace follow {

            using steemit::protocol::base_operation;

            struct follow_operation : base_operation {
                protocol::account_name_type follower;
                protocol::account_name_type following;
                set<string> what; /// blog, mute

                void validate() const;

                void get_required_posting_authorities(flat_set<protocol::account_name_type> &a) const {
                    a.insert(follower);
                }
            };

            struct reblog_operation : base_operation {
                protocol::account_name_type account;
                protocol::account_name_type author;
                string permlink;

                void validate() const;

                void get_required_posting_authorities(flat_set<protocol::account_name_type> &a) const {
                    a.insert(account);
                }
            };

            typedef fc::static_variant<
                    follow_operation,
                    reblog_operation
            > follow_plugin_operation;

            DEFINE_PLUGIN_EVALUATOR(follow_plugin, follow_plugin_operation, follow);

            DEFINE_PLUGIN_EVALUATOR(follow_plugin, follow_plugin_operation, reblog);

        }
    }
} // steemit::follow

FC_REFLECT(steemit::plugins::follow::follow_operation, (follower)(following)(what))
FC_REFLECT(steemit::plugins::follow::reblog_operation, (account)(author)(permlink))

STEEMIT_DECLARE_OPERATION_TYPE(steemit::plugins::follow::follow_plugin_operation)

FC_REFLECT_TYPENAME(steemit::plugins::follow::follow_plugin_operation)
