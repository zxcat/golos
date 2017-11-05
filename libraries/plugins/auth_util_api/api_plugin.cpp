#include <golos/plugins/auth_util_api/api.hpp>
#include <golos/plugins/auth_util_api/api_plugin.hpp>

namespace golos {
    namespace plugins {
        namespace auth_util_api {

            api_plugin::api_plugin() {
            }

            void api_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                api_ptr = std::make_shared<api>();
            }

            void api_plugin::plugin_startup() {
            }

            void api_plugin::plugin_shutdown() {
            }

        }
    }
} // golos::plugins::api
