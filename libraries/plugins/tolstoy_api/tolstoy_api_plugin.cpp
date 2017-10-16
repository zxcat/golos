#include <steemit/plugins/tolstoy_api/tolstoy_api_plugin.hpp>
#include <steemit/plugins/tolstoy_api/tolstoy_api.hpp>
#include <steemit/plugins/chain/chain_plugin.hpp>

namespace steemit { namespace plugins { namespace tolstoy_api {

tolstoy_api_plugin::tolstoy_api_plugin() {}
tolstoy_api_plugin::~tolstoy_api_plugin() {}

void tolstoy_api_plugin::set_program_options( options_description& cli, options_description& cfg ) {
   cli.add_options()
      ("disable-get-block", "Disable get_block API call" );
}

void tolstoy_api_plugin::plugin_initialize( const variables_map& options ) {
   api.reset(new tolstoy_api);
}

void tolstoy_api_plugin::plugin_startup() {

}

void tolstoy_api_plugin::plugin_shutdown() {}

} } } // steem::plugins::condenser_api
