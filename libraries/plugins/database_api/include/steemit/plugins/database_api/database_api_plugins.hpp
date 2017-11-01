#ifndef GOLOS_DATABASE_API_PLUGINS_HPP
#define GOLOS_DATABASE_API_PLUGINS_HPP

#include <steemit/plugins/chain/chain_plugin.hpp>
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>
#include <appbase/application.hpp>
#include <steemit/plugins/database_api/database_api.hpp>

namespace steemit {
    namespace plugins {
        namespace database_api {
            using namespace appbase;
            class database_api_plugin final : public appbase::plugin<database_api_plugin> {
            public:
                constexpr static const char *__name__ = "database_api";
                static const std::string& name() { static std::string name = __name__; return name; }

                database_api_plugin();

                ~database_api_plugin();

                APPBASE_PLUGIN_REQUIRES((json_rpc::json_rpc_plugin) (chain_interface::chain_plugin))

                void set_program_options(
                        options_description &cli,
                        options_description &cfg) override ;

                void plugin_initialize(const variables_map &options) override ;

                void plugin_startup() override ;

                void plugin_shutdown() override ;

                std::shared_ptr<class database_api> api;
            };

        }
    }
} // steemit::plugins::database_api

#endif //GOLOS_DATABASE_API_PLUGINS_HPP
