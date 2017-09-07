#pragma once

#include <steemit/chain/evaluator.hpp>

#include <steemit/plugins/private_message/private_message_operations.hpp>
#include <steemit/plugins/private_message/private_message_plugin.hpp>

namespace steemit {
    namespace plugins {
        namespace private_message {

            //DEFINE_PLUGIN_EVALUATOR(private_message_plugin, private_message_plugin_operation, private_message)
            class private_message_evaluator final : public steemit::chain::evaluator<private_message_evaluator, private_message_plugin_operation> {
            public:
                using operation_type = private_message_operation;

                private_message_evaluator(chain::database &db, private_message_plugin *plugin)
                        : chain::evaluator<private_message_evaluator, private_message_plugin_operation>(db), _plugin(plugin) {}

                void do_apply(const private_message_operation &o);

                private_message_plugin *_plugin;
            };


        }
    }
}
