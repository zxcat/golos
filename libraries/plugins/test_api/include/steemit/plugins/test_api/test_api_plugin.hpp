#pragma once
#include <appbase/application.hpp>

#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>
#include <steemit/plugins/json_rpc/utility.hpp>

namespace steemit { namespace plugins { namespace test_api {

            using namespace appbase;

            struct test_api_a_args {};
            struct test_api_b_args {};

            struct test_api_a_return { std::string value; };
            struct test_api_b_return { std::string value; };

            class test_api_plugin final : public appbase::plugin< test_api_plugin > {
            public:
                test_api_plugin();
                ~test_api_plugin();
                constexpr static const char* __name__="test_api";

                APPBASE_PLUGIN_REQUIRES( (json_rpc::json_rpc_plugin) );

                static const std::string& name() { static std::string name = __name__; return name; }

                virtual void set_program_options( options_description&, options_description& ) override {}

                void plugin_initialize( const variables_map& options ) override;
                void plugin_startup() override;
                void plugin_shutdown() override;

                DECLARE_API(
                (test_api_a)
                        (test_api_b)
                )
            };

        } } } // steem::plugins::test_api

FC_REFLECT( steemit::plugins::test_api::test_api_a_args, )
FC_REFLECT( steemit::plugins::test_api::test_api_b_args, )
FC_REFLECT( steemit::plugins::test_api::test_api_a_return, (value) )
FC_REFLECT( steemit::plugins::test_api::test_api_b_return, (value) )
