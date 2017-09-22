#pragma once
#include <steemit/plugins/follow/follow_plugin.hpp>
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>

#include <appbase/application.hpp>

#define STEEM_FOLLOW_API_PLUGIN_NAME "follow_api"


namespace steemit { namespace plugins { namespace follow_api {

            using namespace appbase;

            class follow_api_plugin final : public appbase::plugin< follow_api_plugin > {
            public:
                APPBASE_PLUGIN_REQUIRES(
                        (follow::follow_plugin)
                        (json_rpc::json_rpc_plugin)
                )

                follow_api_plugin();
                ~follow_api_plugin();
                constexpr static const char *__name__ = "follow_api";
                static const std::string& name() { static std::string name = __name__; return name; }

                void set_program_options( options_description& cli, options_description& cfg ) override;

                void plugin_initialize( const variables_map& options ) override;
                void plugin_startup() override;
                void plugin_shutdown() override;

                std::shared_ptr< class follow_api > api;
            };

        } } } // steem::plugins::follow
