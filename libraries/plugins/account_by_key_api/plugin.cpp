#include <golos/plugins/account_by_key_api/plugin.hpp>
#include <golos/plugins/account_by_key_api/api.hpp>


namespace golos {
    namespace plugins {
        namespace account_by_key {

            plugin::plugin() {
            }

            plugin::~plugin() {
            }

            void plugin::set_program_options(boost::program_options::options_description &cli, boost::program_options::options_description &cfg) {
            }

            void plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                api = std::make_shared<api>();
            }

            void plugin::plugin_startup() {
            }

            void plugin::plugin_shutdown() {
            }

        }
    }
} // golos::plugins::account_by_key