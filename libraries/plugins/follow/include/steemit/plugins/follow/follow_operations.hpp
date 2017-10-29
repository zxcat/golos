#pragma once

#include <steemit/protocol/base.hpp>
#include <steemit/chain/evaluator.hpp>
#include <steemit/plugins/follow/follow_plugin.hpp>

namespace steemit {
    namespace plugins {
        namespace follow {

            struct follow_operation : chain::base_operation<0, 17, 0> {
                protocol::account_name_type follower;
                protocol::account_name_type following;
                std::set<std::string> what; /// blog, mute

                void validate() const;

                void get_required_posting_authorities(flat_set <protocol::account_name_type> &a) const {
                    a.insert(follower);
                }
            };

            struct reblog_operation : chain::base_operation<0, 17, 0> {
                protocol::account_name_type account;
                protocol::account_name_type author;
                std::string permlink;

                void validate() const;

                void get_required_posting_authorities(flat_set <protocol::account_name_type> &a) const {
                    a.insert(account);
                }
            };

            typedef fc::static_variant<follow_operation, reblog_operation> follow_plugin_operation;


        }
    }
} // steemit::follow_api

FC_REFLECT((steemit::plugins::follow::follow_operation), (follower)(following)(what));
FC_REFLECT((steemit::plugins::follow::reblog_operation), (account)(author)(permlink));

namespace fc {

    void to_variant(const steemit::plugins::follow::follow_plugin_operation &, fc::variant &);

    void from_variant(const fc::variant &, steemit::plugins::follow::follow_plugin_operation &);

} /* fc */

namespace steemit {
    namespace protocol {

        void operation_validate(const plugins::follow::follow_plugin_operation &o);

        void operation_get_required_authorities(const plugins::follow::follow_plugin_operation &op,
                                                flat_set <account_name_type> &active,
                                                flat_set <account_name_type> &owner,
                                                flat_set <account_name_type> &posting, vector <authority> &other);

    }
}

FC_REFLECT_TYPENAME((steemit::plugins::follow::follow_plugin_operation));

