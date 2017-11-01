#pragma once

#include <steemit/plugins/auth_util_api/auth_util_api_plugin.hpp>
// 
#include <steemit/plugins/auth_util_api/auth_util_api.hpp>
// 
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>
#include <appbase/application.hpp>
#include <string>

#define STEEMIT_ACCOUNT_BY_KEY_API_PLUGIN_NAME "auth_util_api"

namespace steemit { namespace plugins { namespace auth_util_api {

using namespace appbase;

class auth_util_api_plugin : public appbase::plugin< auth_util_api_plugin > {
public:
	APPBASE_PLUGIN_REQUIRES(
		// (steemit::plugins::auth_util_api::auth_util_api)
		(steemit::plugins::json_rpc::json_rpc_plugin)
	)

	auth_util_api_plugin( ) ;
	virtual ~auth_util_api_plugin() = default;

	static const std::string& name() { static std::string name = STEEMIT_ACCOUNT_BY_KEY_API_PLUGIN_NAME; return name; }

	virtual void set_program_options( options_description& cli, options_description& cfg ) override;

	virtual void plugin_initialize( const variables_map& options ) override;
	virtual void plugin_startup() override;
	virtual void plugin_shutdown() override;

	std::shared_ptr< class auth_util_api > api;
};

} } }
