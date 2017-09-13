#pragma once

#include <steemit/chain/account_object.hpp>
#include <steemit/plugins/chain/chain_plugin.hpp>
#include <appbase/application.hpp>

namespace steemit {
    namespace plugins {
        namespace account_by_key {

            #define ACCOUNT_BY_KEY_PLUGIN_NAME "account_by_key"

            class account_by_key_plugin final : public appbase::plugin<account_by_key_plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES( (steemit::plugins::chain::chain_plugin) )
                static const std::string& name() { static std::string name = ACCOUNT_BY_KEY_PLUGIN_NAME; return name; }
                account_by_key_plugin();
                ~account_by_key_plugin();
                void set_program_options(
                        boost::program_options::options_description &cli,
                        boost::program_options::options_description &cfg) override ;

                void plugin_initialize(const boost::program_options::variables_map &options) override ;

                void plugin_startup()override ;

                void plugin_shutdown()override {}

                void update_key_lookup(const chain::account_authority_object &);

                struct account_by_key_plugin_impl;

                std::unique_ptr<account_by_key_plugin_impl> my;
            };

        }
    }
} // steemit::account_by_key
