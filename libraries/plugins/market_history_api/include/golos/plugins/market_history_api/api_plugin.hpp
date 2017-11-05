#pragma once

#include <golos/plugins/chain/plugin.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>

#include <appbase/application.hpp>

#define STEEMIT_CHAIN_API_PLUGIN_NAME "chain_api"


namespace golos {
    namespace plugins {
        namespace chain {

            using namespace appbase;

            class api_plugin final: public plugin<api_plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES((plugin) (json_rpc::plugin))

                api_plugin();

                virtual ~api_plugin();

                static const std::string &name() {
                    static std::string name = STEEMIT_CHAIN_API_PLUGIN_NAME;
                    return name;
                }

                void set_program_options(boost::program_options::options_description &cli, boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override;

            public:
                class api;

                std::shared_ptr<api> api_ptr;
            };
        }
    }
} // golos::plugins::chain
