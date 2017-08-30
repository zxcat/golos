#pragma once

#include <appbase/application.hpp>
#include <steemit/plugins/block_info/block_info.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <plugins/chain/include/steemit/plugins/chain/chain_plugin.hpp>

namespace steemit {
    namespace protocol {
        struct signed_block;
    }
}

namespace steemit {
    namespace plugins {
        namespace block_info {
            using boost::program_options::options_description;

            class block_info_plugin final : public appbase::plugin<block_info_plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES( (steemit::plugins::chain::chain_plugin) )
                block_info_plugin();

                virtual ~block_info_plugin();

                void set_program_options( options_description& cli, options_description& cfg ){

                }

                virtual void plugin_initialize(const boost::program_options::variables_map &options);

                virtual void plugin_startup();

                virtual void plugin_shutdown();

                void on_applied_block(const chain::signed_block &b);

                std::vector<block_info> _block_info;

                boost::signals2::scoped_connection _applied_block_conn;
            };

        }
    }
}
