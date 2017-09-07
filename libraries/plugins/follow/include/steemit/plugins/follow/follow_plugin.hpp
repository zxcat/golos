#pragma once

#include <steemit/plugins/chain/chain_plugin.hpp>

namespace steemit {
    namespace plugins {
        namespace follow {

            #define FOLLOW_PLUGIN_NAME "follow"

            class follow_plugin : public appbase::plugin<follow_plugin> {
            public:
                constexpr static const char* __name__ = "follow_api";
                APPBASE_PLUGIN_REQUIRES( (steemit::plugins::chain::chain_plugin) )
                follow_plugin();

                void set_program_options(
                        boost::program_options::options_description &cli,
                        boost::program_options::options_description &cfg);

                void plugin_initialize(const boost::program_options::variables_map &options);

                uint32_t max_feed_size();

                void plugin_startup();

                void plugin_shutdown(){}

                ~follow_plugin();

            private:
                struct follow_plugin_impl;

                std::unique_ptr<follow_plugin_impl> my;

            };

        }
    }
} //steemit::follow_api
