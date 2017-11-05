#pragma once

#include <golos/plugins/chain/plugin.hpp>

namespace golos {
    namespace plugins {
        namespace follow {

            class plugin final : public appbase::plugin<plugin> {
            public:

                constexpr static const char *plugin_name = "follow";

                APPBASE_PLUGIN_REQUIRES((chain::plugin))

                static const std::string &name() {
                    static std::string name = plugin_name;
                    return name;
                }

                plugin();

                void set_program_options(boost::program_options::options_description &cli,
                                         boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                uint32_t max_feed_size();

                void plugin_startup() override;

                void plugin_shutdown() override {
                }

                ~plugin();

            private:
                struct follow_plugin_impl;

                std::unique_ptr<follow_plugin_impl> my;

            };

        }
    }
} //golos::follow
