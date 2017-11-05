#pragma once

#include <golos/chain/objects/account_object.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <golos/chain/objects/account_object.hpp>
#include <appbase/application.hpp>

namespace golos {
    namespace plugins {
        namespace account_by_key {

#define ACCOUNT_BY_KEY_PLUGIN_NAME "account_by_key"

            class plugin final : public appbase::plugin<plugin> {
            public:
                APPBASE_PLUGIN_REQUIRES((chain::plugin))

                static const std::string &name() {
                    static std::string name = ACCOUNT_BY_KEY_PLUGIN_NAME;
                    return name;
                }

                plugin();

                ~plugin();

                void set_program_options(boost::program_options::options_description &cli,
                                         boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override;

                void update_key_lookup(const golos::chain::account_authority_object &);

                std::unique_ptr<plugin_impl> my;

            protected:
                struct plugin_impl;
            };
        }
    }
} // golos::account_by_key
