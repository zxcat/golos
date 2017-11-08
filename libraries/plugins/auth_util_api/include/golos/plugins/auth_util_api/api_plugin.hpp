#pragma once

#include <golos/plugins/auth_util_api/api_plugin.hpp>
#include <golos/plugins/auth_util_api/api.hpp>

#include <golos/plugins/json_rpc/plugin.hpp>
#include <appbase/application.hpp>
#include <string>

#define STEEMIT_AUTH_UTIL_API_PLUGIN_NAME "auth_util_api"

namespace golos {
namespace plugins {
namespace auth_util_api {

    using namespace appbase;

    class api;

    class api_plugin : public appbase::plugin<api_plugin> {
    public:
        APPBASE_PLUGIN_REQUIRES((golos::plugins::json_rpc::plugin))

        api_plugin();

        virtual ~api_plugin() = default;

        static const std::string &name() {
            static std::string name = STEEMIT_AUTH_UTIL_API_PLUGIN_NAME;
            return name;
        }

        virtual void set_program_options(boost::program_options::options_description &cli, boost::program_options::options_description &cfg) override;

        virtual void plugin_initialize(const boost::program_options::variables_map &options) override;

        virtual void plugin_startup() override;

        virtual void plugin_shutdown() override;

        std::shared_ptr<api> api_ptr;
    };

}
}
}
