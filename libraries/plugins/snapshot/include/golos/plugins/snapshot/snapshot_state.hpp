#pragma once

#include <fc/time.hpp>
#include <golos/protocol/authority.hpp>

namespace golos {
    namespace plugins {
        namespace snapshot {
            using namespace golos::protocol;
            struct account_keys {
                protocol::authority owner_key;
                protocol::authority active_key;
                protocol::authority posting_key;
                protocol::public_key_type memo_key;
            };

            struct account_balances {
                vector <protocol::asset<0, 17, 0>> assets;
            };

            struct snapshot_summary {
                protocol::asset<0, 17, 0> balance;
                protocol::asset<0, 17, 0> sbd_balance;
                protocol::asset<0, 17, 0> total_vesting_shares;
                protocol::asset<0, 17, 0> total_vesting_fund_steem;
                uint32_t accounts_count;
            };

            struct account_summary {
                uint32_t id;
                string name;
                account_keys keys;
                protocol::share_type posting_rewards;
                protocol::share_type curation_rewards;
                account_balances balances;
                string json_metadata;
                string proxy;
                uint32_t post_count;
                string recovery_account;
                protocol::share_type reputation;
            };

            struct snapshot_state {
                fc::time_point_sec timestamp;
                uint32_t head_block_num;
                protocol::block_id_type head_block_id;
                protocol::chain_id_type chain_id;
                snapshot_summary summary;

                vector <account_summary> accounts;
            };
        }
    }
}

FC_REFLECT((golos::plugins::snapshot::account_keys), (owner_key)(active_key)(posting_key)(memo_key))
FC_REFLECT((golos::plugins::snapshot::account_balances), (assets))
FC_REFLECT((golos::plugins::snapshot::snapshot_summary),
           (balance)(sbd_balance)(total_vesting_shares)(total_vesting_fund_steem)(accounts_count))
FC_REFLECT((golos::plugins::snapshot::account_summary),
           (id)(name)(posting_rewards)(curation_rewards)(keys)(balances)(json_metadata)(proxy)(post_count)(
                   recovery_account)(reputation))
FC_REFLECT((golos::plugins::snapshot::snapshot_state),
           (timestamp)(head_block_num)(head_block_id)(chain_id)(summary)(accounts))