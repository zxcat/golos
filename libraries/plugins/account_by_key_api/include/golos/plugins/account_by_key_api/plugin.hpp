#pragma once

#include <golos/plugins/account_by_key/plugin.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>

#include <appbase/application.hpp>

#define STEEMIT_ACCOUNT_BY_KEY_API_PLUGIN_NAME "account_by_key_api"


namespace golos {
    namespace plugins {
        namespace account_by_key {

            using namespace appbase;

            class plugin : public appbase::plugin<plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES((golos::plugins::account_by_key::plugin) (golos::plugins::json_rpc::plugin))

                plugin();

                virtual ~plugin();

                static const std::string &name() {
                    static std::string name = STEEMIT_ACCOUNT_BY_KEY_API_PLUGIN_NAME;
                    return name;
                }

                virtual void set_program_options(boost::program_options::options_description &cli,
                                                 boost::program_options::options_description &cfg) override;

                virtual void plugin_initialize(const boost::program_options::variables_map &options) override;

                virtual void plugin_startup() override;

                virtual void plugin_shutdown() override;

            protected:
                class api;

                std::shared_ptr<api> api_ptr;
            };
        }
    }
} // golos::plugins::account_by_key
