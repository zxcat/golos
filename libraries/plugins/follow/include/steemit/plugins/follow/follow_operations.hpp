#pragma once

#include <steemit/protocol/base.hpp>
#include <steemit/chain/evaluator.hpp>
#include <steemit/plugins/follow/follow_plugin.hpp>

namespace steemit {
    namespace plugins {
        namespace follow {

            #define DEFINE_PLUGIN_EVALUATOR(PLUGIN, OPERATION, X)                     \
            class X ## _evaluator : public steemit::chain::evaluator< X ## _evaluator, OPERATION > \
            {                                                                           \
               public:                                                                  \
                  typedef X ## _operation operation_type;                               \
                                                                                        \
                  X ## _evaluator( chain::database& db, PLUGIN* plugin )                       \
                     : steemit::chain::evaluator< X ## _evaluator, OPERATION >( db ), \
                       _plugin( plugin )                                                \
                  {}                                                                    \
                                                                                        \
                  void do_apply( const X ## _operation& o );                            \
                                                                                        \
                  PLUGIN* _plugin;                                                      \
            };

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
} // steemit::follow_api

FC_REFLECT(steemit::plugins::follow::follow_operation, (follower)(following)(what))
FC_REFLECT(steemit::plugins::follow::reblog_operation, (account)(author)(permlink))

STEEMIT_DECLARE_OPERATION_TYPE(steemit::plugins::follow::follow_plugin_operation)

FC_REFLECT_TYPENAME(steemit::plugins::follow::follow_plugin_operation)
