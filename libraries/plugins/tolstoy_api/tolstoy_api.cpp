#include <steemit/plugins/tolstoy_api/tolstoy_api.hpp>
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>

namespace steemit {
    namespace plugins {
        namespace tolstoy_api {

            using fc::variant;

            struct tolstoy_api::tolstoy_api_impl {
            public:
                tolstoy_api_impl() : rpc(appbase::app().get_plugin<json_rpc::json_rpc_plugin>()) {}

                variant send(const std::string &api_namespace,
                             const std::string &name_method,
                             std::vector<variant> body) {
                    return rpc.rpc(api_namespace, name_method, body);
                }

            private:
                json_rpc::json_rpc_plugin &rpc;
            };

            tolstoy_api::tolstoy_api() : pimpl(new tolstoy_api_impl) {
                JSON_RPC_REGISTER_API(
                        __name__,
                        (get_trending_tags)

                                (get_state)

                                (get_trending_categories)

                                (get_best_categories)

                                (get_active_categories)

                                (get_recent_categories)

                                (get_active_witnesses)

                                (get_miner_queue)

                                (get_block_header)

                                (get_block)

                                (get_ops_in_block)

                                (get_config)

                                (get_dynamic_global_properties)

                                (get_chain_properties)

                                (get_current_median_history_price)

                                (get_feed_history)

                                (get_witness_schedule)

                                (get_hardfork_version)

                                (get_next_scheduled_hardfork)

                                (get_reward_fund)

                                (get_accounts)

                                (lookup_account_names)

                                (lookup_accounts)

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

                                (get_witnesses)

                                (get_conversion_requests)

                                (get_witness_by_account)

                                (get_witnesses_by_vote)

                                (lookup_witness_accounts)

                                (get_witness_count)

                                (get_assets)

                                (get_assets_by_issuer)

                                (get_assets_dynamic_data)

                                (get_bitassets_data)

                                (list_assets)

                                (lookup_asset_symbols)

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

                                (get_proposed_transactions)

                )

            }

            tolstoy_api::~tolstoy_api() {

            }






        }
    }
}

#endif //GOLOS_TOLSTOY_API_PLUGIN_HPP
