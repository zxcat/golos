#pragma once

#include <golos/chain/objects/account_object.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <golos/chain/objects/account_object.hpp>
#include <appbase/application.hpp>
#include <golos/plugins/json_rpc/utility.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>
namespace golos {
    namespace plugins {
        namespace account_by_key {


            using get_key_references_args = json_rpc::msg_pack;

            struct get_key_references_return {
                std::vector<std::vector<golos::protocol::account_name_type> > accounts;
            };

#define ACCOUNT_BY_KEY_PLUGIN_NAME "account_by_key"

            class plugin final : public appbase::plugin<plugin> {
            private:
                class plugin_impl;
            public:
                APPBASE_PLUGIN_REQUIRES((chain::plugin)(json_rpc::plugin))

                static const std::string &name() {
                    static std::string name = ACCOUNT_BY_KEY_PLUGIN_NAME;
                    return name;
                }

                DECLARE_API((get_key_references))

                plugin();

                ~plugin();

                void set_program_options(boost::program_options::options_description &cli,
                                         boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override;

                void update_key_lookup(const golos::chain::account_authority_object &);

                std::unique_ptr<plugin_impl> my;
            };
        }
    }
} // golos::account_by_key

FC_REFLECT((golos::plugins::account_by_key::get_key_references_return), (accounts))
