#ifndef GOLOS_TOLSTOY_API_PLUGIN_HPP
#define GOLOS_TOLSTOY_API_PLUGIN_HPP

#include <appbase/plugin.hpp>
#include <steemit/plugins/chain/chain_plugin.hpp>
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>
#include <memory>


namespace steemit {
    namespace plugins {
        namespace tolstoy_api {
            using namespace  appbase;
            class tolstoy_api_plugin final : public appbase::plugin<tolstoy_api_plugin> {
            public:
                tolstoy_api_plugin();
                ~tolstoy_api_plugin();
                constexpr static const char* __name__ = "tolstoy_api";
                APPBASE_PLUGIN_REQUIRES(
                        (json_rpc::json_rpc_plugin)
                        (chain::chain_plugin)
                );

                static const std::string& name() { static std::string name = __name__; return name; }

                void set_program_options( options_description&, options_description& ) override;

                void plugin_initialize( const variables_map& options ) override;
                void plugin_startup() override;
                void plugin_shutdown() override;

                std::shared_ptr<class tolstoy_api> pimpl;
            };


        }

    }
}
#endif //GOLOS_TOLSTOY_API_PLUGIN_HPP
