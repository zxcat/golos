
#include <steemit/plugins/raw_block/raw_block_plugin.hpp>

namespace steemit {
    namespace plugin {
        namespace raw_block {

            raw_block_plugin::raw_block_plugin() {
                name("raw_block");
            }

            raw_block_plugin::~raw_block_plugin() {
            }

            void raw_block_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
            }

            void raw_block_plugin::plugin_startup() {
            }

            void raw_block_plugin::plugin_shutdown() {
            }

        }
    }
} // steemit::plugin::raw_block
