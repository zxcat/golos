#pragma once

#include <appbase/application.hpp>
#include <golos/plugins/block_info/block_info.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <golos/plugins/chain/plugin.hpp>

namespace golos {
    namespace protocol {
        struct signed_block;
    }
}

namespace golos {
    namespace plugins {
        namespace block_info {
            using boost::program_options::options_description;

            class plugin final : public appbase::plugin<plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES((golos::plugins::chain::plugin))

                constexpr const static char *plugin_name = "block_info";

                static const std::string &name() {
                    static std::string name = plugin_name;
                    return name;
                }

                plugin();

                ~plugin();

                void set_program_options(boost::program_options::options_description &cli, boost::program_options::options_description &cfg) override {
                }

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override;

                void on_applied_block(const protocol::signed_block &b);

                std::vector<block_info> _block_info;

                boost::signals2::scoped_connection _applied_block_conn;
            };
        }
    }
}
