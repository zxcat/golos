#ifndef GOLOS_FOLLOW_EVALUATORS_HPP
#define GOLOS_FOLLOW_EVALUATORS_HPP

#include <golos/plugins/follow/follow_operations.hpp>

#include <golos/chain/evaluator.hpp>

namespace golos {
    namespace plugins {
        namespace follow {
            class follow_evaluator : public golos::chain::evaluator<follow_evaluator, 0, 17, 0,
                    follow_plugin_operation> {
            public:
                typedef follow_operation operation_type;

                follow_evaluator(chain::database &db, plugin *plugin) : golos::chain::evaluator<
                        follow_evaluator, 0, 17, 0, follow_plugin_operation>(db), _plugin(plugin) {
                }

                void do_apply(const follow_operation &o);

                plugin *_plugin;
            };

            class reblog_evaluator : public golos::chain::evaluator<reblog_evaluator, 0, 17, 0,
                    follow_plugin_operation> {
            public:
                typedef reblog_operation operation_type;

                reblog_evaluator(chain::database &db, plugin *plugin) : golos::chain::evaluator<
                        reblog_evaluator, 0, 17, 0, follow_plugin_operation>(db), _plugin(plugin) {
                }

                void do_apply(const reblog_operation &o);

                plugin *_plugin;
            };
        }
    }
}

#endif //GOLOS_FOLLOW_EVALUATORS_HPP
