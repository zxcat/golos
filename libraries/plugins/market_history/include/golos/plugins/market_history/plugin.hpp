#pragma once

#include <golos/chain/steem_object_types.hpp>

#include <golos/plugins/market_history/bucket_object.hpp>
#include <golos/plugins/market_history/order_history_object.hpp>
#include <appbase/application.hpp>
#include <golos/plugins/chain/plugin.hpp>

#ifndef MARKET_HISTORY_PLUGIN_NAME
#define MARKET_HISTORY_PLUGIN_NAME "market_history"
#endif


namespace golos {
    namespace plugins {
        namespace market_history {

            using namespace chain;

            /**
             *  The market history plugin can be configured to track any number of intervals via its configuration.  Once         per block it
             *  will scan the virtual operations and look for fill_order and fill_asset_order operations and then adjust the appropriate bucket objects for
             *  each fill order.
             */

            class plugin final : public appbase::plugin<plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES((chain::plugin))

                static const std::string &name() {
                    static std::string name = MARKET_HISTORY_PLUGIN_NAME;
                    return name;
                }

                plugin();

                ~plugin();

                void set_program_options(boost::program_options::options_description &cli,
                                         boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override {
                }

                flat_set<uint32_t> get_tracked_buckets() const;

                uint32_t get_max_history_per_bucket() const;

            private:
                class plugin_impl;

                std::unique_ptr<plugin_impl> impl;
            };
        }
    }
} // golos::market_history
