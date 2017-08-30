#pragma once

#include <steemit/chain/steem_objects.hpp>
#include <appbase/application.hpp>

namespace steemit {
    namespace delayed_node {
        namespace detail { struct delayed_node_plugin_impl; }

        class delayed_node_plugin : public appbase::plugin<delayed_node_plugin> {
            std::unique_ptr<detail::delayed_node_plugin_impl> my;
        public:
            delayed_node_plugin();

            ~delayed_node_plugin();

            void plugin_set_program_options(boost::program_options::options_description &,
                    boost::program_options::options_description &cfg);

            void plugin_initialize(const boost::program_options::variables_map &options);

            void plugin_startup();

            void mainloop();

        protected:
            void connection_failed();

            void connect();

            void sync_with_trusted_node();
        };

    }
} //steemit::account_history

