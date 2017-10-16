#pragma once
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>
#include <steemit/plugins/database_api/database_api_plugins.hpp>
#include <memory>
namespace steemit { namespace plugins { namespace tolstoy_api {

using namespace appbase;

class tolstoy_api_plugin final : public appbase::plugin< tolstoy_api_plugin > {
public:
   APPBASE_PLUGIN_REQUIRES( (json_rpc::json_rpc_plugin)(database_api::database_api_plugin) )

   tolstoy_api_plugin();
   ~tolstoy_api_plugin();
   constexpr static const char* __name__ = "tolstoy_api";
   static const std::string& name() { static std::string name = __name__; return name; }

   void set_program_options( options_description& cli, options_description& cfg ) override;

   void plugin_initialize( const variables_map& options ) override;
   void plugin_startup() override;
   void plugin_shutdown() override;
private:
   std::unique_ptr< class tolstoy_api > api;
};

} } } // steem::plugins::condenser_api
