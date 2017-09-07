#ifndef GOLOS_FORWARD_HPP
#define GOLOS_FORWARD_HPP

#include <chain/include/steemit/chain/steem_objects.hpp>

namespace steemit {
    namespace plugins {
        namespace database_api {
            typedef chain::change_recovery_account_request_object change_recovery_account_request_api_object;
            typedef chain::block_summary_object block_summary_api_object;
            typedef chain::comment_vote_object comment_vote_api_object;
            typedef chain::dynamic_global_property_object dynamic_global_property_api_object;
            typedef chain::convert_request_object convert_request_api_object;
            typedef chain::escrow_object escrow_api_object;
            typedef chain::liquidity_reward_balance_object liquidity_reward_balance_api_object;
            typedef chain::limit_order_object limit_order_api_object;
            typedef chain::withdraw_vesting_route_object withdraw_vesting_route_api_object;
            typedef chain::decline_voting_rights_request_object decline_voting_rights_request_api_object;
            typedef chain::witness_vote_object witness_vote_api_object;
            typedef chain::witness_schedule_object witness_schedule_api_object;
            typedef chain::account_bandwidth_object account_bandwidth_api_object;
            typedef chain::vesting_delegation_object vesting_delegation_api_object;
            typedef chain::vesting_delegation_expiration_object vesting_delegation_expiration_api_object;
            typedef chain::reward_fund_object reward_fund_api_object;
        }}}
#endif //GOLOS_FORWARD_HPP
