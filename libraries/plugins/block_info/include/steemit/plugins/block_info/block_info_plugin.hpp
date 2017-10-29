#pragma once

#include <appbase/application.hpp>
#include <steemit/plugins/block_info/block_info.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <steemit/plugins/chain/chain_plugin.hpp>

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
                APPBASE_PLUGIN_REQUIRES( (steemit::plugins::chain_interface::chain_plugin) )
                constexpr const static char* __name__="block_info";
                static const std::string& name() { static std::string name = __name__; return name; }
                block_info_plugin();

                ~block_info_plugin();

                void set_program_options( options_description& cli, options_description& cfg )override {}

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup()override;

                void plugin_shutdown()override;

                void on_applied_block(const protocol::signed_block &b);

                std::vector<block_info> _block_info;

                boost::signals2::scoped_connection _applied_block_conn;
            };

        }
    }
}
