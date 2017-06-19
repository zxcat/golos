#pragma once

#include <steemit/application/plugin.hpp>
#include <steemit/chain/database.hpp>

#include <fc/thread/future.hpp>

#include <steemit/follow/follow_api.hpp>

namespace steemit {
    namespace follow {
        using steemit::application::application;

#define FOLLOW_PLUGIN_NAME "follow"

        class follow_plugin : public steemit::application::plugin {
        public:
            follow_plugin(application *app);

            std::string plugin_name() const override {
                return FOLLOW_PLUGIN_NAME;
            }

            virtual void plugin_set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg) override;

            virtual void plugin_initialize(const boost::program_options::variables_map &options) override;

            virtual void plugin_startup() override;

            struct follow_plugin_impl;

            std::unique_ptr<follow_plugin_impl> my;
            uint32_t max_feed_size = 500;
        };

    }
} //steemit::follow
