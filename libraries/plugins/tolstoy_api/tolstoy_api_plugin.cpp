#include <steemit/plugins/tolstoy_api/tolstoy_api_plugin.hpp>
#include <steemit/plugins/tolstoy_api/tolstoy_api.hpp>
namespace steemit {
    namespace plugins {
        namespace tolstoy_api {

            tolstoy_api_plugin::tolstoy_api_plugin():pimpl(new tolstoy_api) {
            }

            tolstoy_api_plugin::~tolstoy_api_plugin() {

            }

            void tolstoy_api_plugin::plugin_startup() {

            }

            void tolstoy_api_plugin::plugin_shutdown() {

            }

            void tolstoy_api_plugin::plugin_initialize( const variables_map& options ) {

            }

            void tolstoy_api_plugin::set_program_options(options_description &, options_description &) {

            }
        }
    }
}