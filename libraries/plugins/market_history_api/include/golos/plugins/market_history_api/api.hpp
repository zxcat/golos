#pragma once

#include <golos/plugins/json_rpc/utility.hpp>

#include <golos/protocol/types.hpp>

#include <fc/optional.hpp>

#define STEEMIT_MARKET_HISTORY_API_PLUGIN_NAME "market_history_api"

namespace golos {
    namespace plugins {
        namespace market_history {

            struct market_ticker {
                string base;
                string quote;
                double latest;
                double lowest_ask;
                double highest_bid;
                double percent_change;
                double base_volume;
                double quote_volume;
            };

            struct market_volume {
                string base;
                string quote;
                double base_volume;
                double quote_volume;
            };

            struct order {
                double price;
                double quote;
                double base;
            };

            struct order_book {
                string base;
                string quote;
                std::vector<order> bids;
                std::vector<order> asks;
            };

            struct market_trade {
                fc::time_point_sec date;
                double price;
                double amount;
                double value;
            };

            struct liquidity_balance {
                std::string account;
                fc::uint128_t weight;
            };

            DEFINE_API_ARGS(get_limit_orders_by_owner, msg_pack, std::vector<golos::application::extended_limit_order>);
            DEFINE_API_ARGS(get_call_orders_by_owner, msg_pack, std::vector<call_order_object>);
            DEFINE_API_ARGS(get_settle_orders_by_owner, msg_pack, std::vector<force_settlement_object>);
            DEFINE_API_ARGS(get_ticker, msg_pack, market_ticker);
            DEFINE_API_ARGS(get_volume, msg_pack, market_volume);
            DEFINE_API_ARGS(get_order_book, msg_pack, order_book);
            DEFINE_API_ARGS(get_trade_history, msg_pack, std::vector<market_trade>);
            DEFINE_API_ARGS(get_fill_order_history, msg_pack, vector<order_history_object>);
            DEFINE_API_ARGS(get_market_history, msg_pack, vector<bucket_object>);
            DEFINE_API_ARGS(get_market_history_buckets, void_type, flat_set<uint32_t>);
            DEFINE_API_ARGS(get_limit_orders, msg_pack, vector<limit_order_object>);
            DEFINE_API_ARGS(get_call_orders, msg_pack, vector<call_order_object>);
            DEFINE_API_ARGS(get_settle_orders, msg_pack, vector<force_settlement_object>);
            DEFINE_API_ARGS(get_collateral_bids, msg_pack, vector<collateral_bid_object>);
            DEFINE_API_ARGS(get_margin_positions, msg_pack, vector<call_order_object>);
            DEFINE_API_ARGS(get_liquidity_queue, msg_pack, std::vector<liquidity_balance>);

            class api {
            public:
                api();

                ~api();

                ///////////////////
                // Subscriptions //
                ///////////////////

                void set_subscribe_callback(std::function<void( const variant &)> cb, bool clear_filter);

                void set_pending_transaction_callback(std::function<void( const variant &)> cb);

                void set_block_applied_callback(std::function<void( const variant &block_header)> cb);

                /**
                 * @brief Stop receiving any notifications
                 *
                 * This unsubscribes from all subscribed markets and objects.
                 */
                void cancel_all_subscriptions();

                /**
                 * @brief Request notification when the active orders in the market between two assets changes
                 * @param callback Callback method which is called when the market changes
                 * @param a First asset ID
                 * @param b Second asset ID
                 *
                 * Callback will be passed a variant containing a vector<pair<operation, operation_result>>. The vector will
                 * contain, in order, the operations which changed the market, and their results.
             */
                void subscribe_to_market(std::function<void( const variant &)> callback, const string &a, const string &b);

                /**
                 * @brief Unsubscribe from updates to a given market
                 * @param a First asset ID
                 * @param b Second asset ID
                 */
                void unsubscribe_from_market(const string &a, const string &b);

                DECLARE_API(
//                (set_subscribe_callback)(set_pending_transaction_callback)(set_block_applied_callback)(cancel_all_subscriptions)

               //Market
                       (get_ticker)
                       (get_volume)
                       (get_order_book)
                       (get_trade_history)
                       (get_market_history)
                       (get_market_history_buckets)
                       (get_limit_orders)
                       (get_call_orders)
                       (get_settle_orders)
                       (get_margin_positions)
                       (get_liquidity_queue)
                       (subscribe_to_market)
                       (unsubscribe_from_market)
                       (get_limit_orders_by_owner)
                       (get_call_orders_by_owner)
                       (get_settle_orders_by_owner)
                       (get_fill_order_history)
                       (get_collateral_bids)
                )

            private:
                class api_impl;

                std::unique_ptr<api_impl> my;
            };

        }
    }
} // golos::plugins::chain

FC_REFLECT((golos::plugins::market_history::order), (price)(quote)(base));
FC_REFLECT((golos::plugins::market_history::order_book), (asks)(bids)(base)(quote));
FC_REFLECT((golos::plugins::market_history::market_ticker),
           (base)(quote)(latest)(lowest_ask)(highest_bid)(percent_change)(base_volume)(quote_volume));
FC_REFLECT((golos::plugins::market_history::market_volume), (base)(quote)(base_volume)(quote_volume));
FC_REFLECT((golos::plugins::market_history::market_trade), (date)(price)(amount)(value));

FC_REFLECT((golos::plugins::market_history::liquidity_balance), (account)(weight));
