#ifndef GOLOS_DATABASE_API_PLUGINS_HPP
#define GOLOS_DATABASE_API_PLUGINS_HPP

#include <golos/plugins/chain/plugin.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>
#include <appbase/application.hpp>
#include <golos/plugins/database_api/api.hpp>

namespace golos {
    namespace plugins {
        namespace database_api {
            using namespace appbase;

            class api;

            class api_plugin final : public appbase::plugin<api_plugin> {
            public:
                constexpr static const char *plugin_name = "api";

                static const std::string &name() {
                    static std::string name = plugin_name;
                    return name;
                }

                api_plugin();

                ~api_plugin();

                APPBASE_PLUGIN_REQUIRES((json_rpc::plugin)(chain::plugin))

                void set_program_options(boost::program_options::options_description &cli, boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override;

                std::shared_ptr<api> api_ptr;
            };
        }
    }
} // golos::plugins::api

#endif //GOLOS_DATABASE_API_PLUGINS_HPP
