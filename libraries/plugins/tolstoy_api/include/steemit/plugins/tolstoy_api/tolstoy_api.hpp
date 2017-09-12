#ifndef GOLOS_TOLSTOY_API_PLUGIN_HPP
#define GOLOS_TOLSTOY_API_PLUGIN_HPP

#include <steemit/plugins/json_rpc/utility.hpp>
#include <fc/variant.hpp>


namespace steemit {
    namespace plugins {
        namespace tolstoy_api {
            using fc::variant;
            using std::vector;
            using plugins::json_rpc::void_type;

            DEFINE_API_ARGS( get_trending_tags,                      vector< variant >,   variant)
            DEFINE_API_ARGS( get_state,                              vector< variant >,   variant)
            DEFINE_API_ARGS( get_active_witnesses,                   void_type,           variant)
            DEFINE_API_ARGS( get_block_header,                       vector< variant >,   variant)
            DEFINE_API_ARGS( get_block,                              vector< variant >,   variant)
            DEFINE_API_ARGS( get_ops_in_block,                       vector< variant >,   variant)
            DEFINE_API_ARGS( get_config,                             void_type,           variant)
            DEFINE_API_ARGS( get_dynamic_global_properties,          void_type,           variant)
            DEFINE_API_ARGS( get_chain_properties,                   void_type,           variant)
            DEFINE_API_ARGS( get_current_median_history_price,       void_type,           variant)
            DEFINE_API_ARGS( get_feed_history,                       void_type,           variant)
            DEFINE_API_ARGS( get_witness_schedule,                   void_type,           variant)
            DEFINE_API_ARGS( get_hardfork_version,                   void_type,           variant)
            DEFINE_API_ARGS( get_next_scheduled_hardfork,            void_type,           variant)
            DEFINE_API_ARGS( get_reward_fund,                        vector< variant >,   variant)
            DEFINE_API_ARGS( get_key_references,                     vector< variant >,   variant)
            DEFINE_API_ARGS( get_accounts,                           vector< variant >,   variant)
            DEFINE_API_ARGS( lookup_account_names,                   vector< variant >,   variant)
            DEFINE_API_ARGS( lookup_accounts,                        vector< variant >,   variant)
            DEFINE_API_ARGS( get_account_count,                      void_type,           variant)
            DEFINE_API_ARGS( get_owner_history,                      vector< variant >,   variant)
            DEFINE_API_ARGS( get_recovery_request,                   vector< variant >,   variant)
            DEFINE_API_ARGS( get_escrow,                             vector< variant >,   variant)
            DEFINE_API_ARGS( get_withdraw_routes,                    vector< variant >,   variant)
            DEFINE_API_ARGS( get_account_bandwidth,                  vector< variant >,   variant)
            DEFINE_API_ARGS( get_savings_withdraw_from,              vector< variant >,   variant)
            DEFINE_API_ARGS( get_savings_withdraw_to,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_vesting_delegations,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_expiring_vesting_delegations,       vector< variant >,   variant)
            DEFINE_API_ARGS( get_witnesses,                          vector< variant >,   variant)
            DEFINE_API_ARGS( get_conversion_requests,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_witness_by_account,                 vector< variant >,   variant)
            DEFINE_API_ARGS( get_witnesses_by_vote,                  vector< variant >,   variant)
            DEFINE_API_ARGS( lookup_witness_accounts,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_open_orders,                        vector< variant >,   variant)
            DEFINE_API_ARGS( get_witness_count,                      void_type,           variant)
            DEFINE_API_ARGS( get_transaction_hex,                    vector< variant >,   variant)
            DEFINE_API_ARGS( get_transaction,                        vector< variant >,   variant)
            DEFINE_API_ARGS( get_required_signatures,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_potential_signatures,               vector< variant >,   variant)
            DEFINE_API_ARGS( verify_authority,                       vector< variant >,   variant)
            DEFINE_API_ARGS( verify_account_authority,               vector< variant >,   variant)
            DEFINE_API_ARGS( get_active_votes,                       vector< variant >,   variant)
            DEFINE_API_ARGS( get_account_votes,                      vector< variant >,   variant)
            DEFINE_API_ARGS( get_content,                            vector< variant >,   variant)
            DEFINE_API_ARGS( get_content_replies,                    vector< variant >,   variant)
            DEFINE_API_ARGS( get_tags_used_by_author,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_payout,              vector< variant >,   variant)
            DEFINE_API_ARGS( get_post_discussions_by_payout,         vector< variant >,   variant)
            DEFINE_API_ARGS( get_comment_discussions_by_payout,      vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_trending,            vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_created,             vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_active,              vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_cashout,             vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_votes,               vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_children,            vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_hot,                 vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_feed,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_blog,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_comments,            vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_promoted,            vector< variant >,   variant)
            DEFINE_API_ARGS( get_replies_by_last_update,             vector< variant >,   variant)
            DEFINE_API_ARGS( get_discussions_by_author_before_date,  vector< variant >,   variant)
            DEFINE_API_ARGS( get_account_history,                    vector< variant >,   variant)
            ///asset
            DEFINE_API_ARGS( get_assets_dynamic_data,                vector< variant >,   variant)
            DEFINE_API_ARGS( get_assets_by_issuer,                   vector< variant >,   variant)
            ///categories
            DEFINE_API_ARGS( get_trending_categories,                vector< variant >,   variant)

            ///
            DEFINE_API_ARGS(get_account_balances,                    vector< variant >,   variant)
            DEFINE_API_ARGS(get_active_categories,                   vector< variant >,   variant)
            DEFINE_API_ARGS(get_recent_categories,                   vector< variant >,   variant)
            DEFINE_API_ARGS(get_proposed_transactions,               vector< variant >,   variant)
            DEFINE_API_ARGS(list_assets,                             vector< variant >,   variant)
            DEFINE_API_ARGS(get_bitassets_data,                      vector< variant >,   variant)
            DEFINE_API_ARGS(get_miner_queue,                         vector< variant >,   variant)
            DEFINE_API_ARGS(get_assets,                              vector< variant >,   variant)
            DEFINE_API_ARGS(get_best_categories,                     vector< variant >,   variant)
            DEFINE_API_ARGS(lookup_asset_symbols,                    vector< variant >,   variant)
            DEFINE_API_ARGS(get_payout_extension_cost,               vector< variant >,   variant)
            DEFINE_API_ARGS(get_payout_extension_time,               vector< variant >,   variant)

            class tolstoy_api final {

                constexpr static const char* __name__ = "tolstoy_api";

                tolstoy_api();

                ~tolstoy_api();

                DECLARE_API(
                        (get_trending_tags)

                        (get_state)

                        (get_trending_categories)

                        (get_best_categories)

                        (get_active_categories)

                        (get_recent_categories)

                        (get_active_witnesses)

                        (get_miner_queue)

                /////////////////////////////
                // Blocks and transactions //
                /////////////////////////////

                        (get_block_header)

                        (get_block)

                        (get_ops_in_block)

                /////////////
                // Globals //
                /////////////

                        (get_config)

                        (get_dynamic_global_properties)

                        (get_chain_properties)

                        (get_current_median_history_price)

                        (get_feed_history)

                        (get_witness_schedule)

                        (get_hardfork_version)

                        (get_next_scheduled_hardfork)

                        (get_reward_fund)

                //////////////
                // Accounts //
                //////////////

                        (get_accounts)

                        (lookup_account_names)

                        (lookup_accounts)

                //////////////
                // Balances //
                //////////////

                        (get_account_balances)

                        (get_account_count)

                        (get_owner_history)

                        (get_recovery_request)

                        (get_escrow)

                        (get_withdraw_routes)

                        (get_account_bandwidth)

                        (get_savings_withdraw_from)

                        (get_savings_withdraw_to)

                        (get_vesting_delegations)

                        (get_expiring_vesting_delegations)

                ///////////////
                // Witnesses //
                ///////////////

                        (get_witnesses)

                        (get_conversion_requests)

                        (get_witness_by_account)

                        (get_witnesses_by_vote)

                        (lookup_witness_accounts)

                        (get_witness_count)

                ////////////
                // Assets //
                ////////////

                        (get_assets)

                        (get_assets_by_issuer)

                        (get_assets_dynamic_data)

                        (get_bitassets_data)

                        (list_assets)

                        (lookup_asset_symbols)

                ////////////////////////////
                // Authority / Validation //
                ////////////////////////////

                        (get_transaction_hex)

                        (get_transaction)

                        (get_required_signatures)

                        (get_potential_signatures)

                        (verify_authority)

                        (verify_account_authority)

                        (get_active_votes)

                        (get_account_votes)

                        (get_content)

                        (get_content_replies)

                        (get_tags_used_by_author)

                        (get_discussions_by_trending)

                        (get_discussions_by_created)

                        (get_discussions_by_active)

                        (get_discussions_by_cashout)

                        (get_discussions_by_payout)

                        (get_post_discussions_by_payout)

                        (get_comment_discussions_by_payout)

                        (get_discussions_by_votes)

                        (get_discussions_by_children)

                        (get_discussions_by_hot)

                        (get_discussions_by_feed)

                        (get_discussions_by_blog)

                        (get_discussions_by_comments)

                        (get_discussions_by_promoted)

                        (get_replies_by_last_update)

                        (get_discussions_by_author_before_date)

                        (get_account_history)

                        (get_payout_extension_cost)

                        (get_payout_extension_time)

                ///////////////////////////
                // Proposed transactions //
                ///////////////////////////

                        (get_proposed_transactions)

                )

            private:
                struct tolstoy_api_impl;
                std::unique_ptr<tolstoy_api_impl> pimpl;
            };
        }
    }
}

#endif //GOLOS_TOLSTOY_API_PLUGIN_HPP
