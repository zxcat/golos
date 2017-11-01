#ifndef GOLOS_SNAPSHOT_PLUGIN_HPP
#define GOLOS_SNAPSHOT_PLUGIN_HPP


#include <appbase/application.hpp>

#include <boost/bimap.hpp>

#include <sstream>
#include <string>
#include <steemit/plugins/chain/chain_plugin.hpp>

#define SNAPSHOT_PLUGIN_NAME "snapshot"

namespace steemit {
    namespace plugins {
        namespace snapshot {

            class snapshot_plugin final : public appbase::plugin<snapshot_plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES( (chain_interface::chain_plugin) )
                static const std::string& name() { static std::string name = SNAPSHOT_PLUGIN_NAME; return name; }
                /**
                 * The plugin requires a constructor which takes app.  This is called regardless of whether the plugin is loaded.
                 * The app parameter should be passed up to the superclass constructor.
                 */
                snapshot_plugin();

                /**
                 * Plugin is destroyed via base class pointer, so a virtual destructor must be provided.
                 */
                ~snapshot_plugin();


                /**
                 * Called when the plugin is enabled, but before the database has been created.
                 */
                void plugin_initialize(const boost::program_options::variables_map &options) override ;

                void set_program_options(
                        boost::program_options::options_description &command_line_options,
                        boost::program_options::options_description &config_file_options) override ;

                /**
                 * Called when the plugin is enabled.
                 */
                void plugin_startup() override ;

                void plugin_shutdown() override {}

                const boost::bimap<std::string, std::string> &get_loaded_snapshots() const;

            private:

                struct snapshot_plugin_impl;

                std::unique_ptr<snapshot_plugin_impl> impl;
            };
        }
    }
}

#endif //GOLOS_SNAPSHOT_PLUGIN_HPP