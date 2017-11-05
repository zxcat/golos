#pragma once

#include <golos/plugins/follow/plugin.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>

#include <appbase/application.hpp>

#define STEEM_FOLLOW_API_PLUGIN_NAME "follow_api"


namespace golos {
    namespace plugins {
        namespace follow {

        class api;

            using namespace appbase;

            class api_plugin final : public appbase::plugin<api_plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES((follow::plugin) (json_rpc::plugin))

                api_plugin();

                ~api_plugin();

                constexpr static const char *plugin_name = "follow";

                static const std::string &name() {
                    static std::string name = plugin_name;
                    return name;
                }

                void set_program_options(boost::program_options::options_description &cli, boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override;

                std::shared_ptr<api> api_ptr;
            };

        }
    }
} // steem::plugins::follow
