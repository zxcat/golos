
#pragma once

#include <appbase/application.hpp>

namespace steemit {
    namespace plugin {
        namespace raw_block {

            class raw_block_plugin : public appbase::plugin<raw_block_plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES()
                raw_block_plugin();

                virtual ~raw_block_plugin();

                void plugin_initialize(const boost::program_options::variables_map &options) ;

                void plugin_startup();

                void plugin_shutdown();
            };

        }
    }
}
