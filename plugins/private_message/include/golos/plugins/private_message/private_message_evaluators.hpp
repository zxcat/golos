#pragma once

#include <golos/plugins/private_message/private_message_objects.hpp>
#include <golos/plugins/private_message/private_message_plugin.hpp>
#include <golos/chain/database.hpp>
#include <golos/chain/evaluator.hpp>

namespace golos { namespace plugins { namespace private_message {

    using golos::chain::evaluator_impl;

    class private_message_evaluator:
        public evaluator_impl<private_message_evaluator, private_message_plugin_operation>
    {
    public:
        using operation_type = private_message_operation;

        private_message_evaluator(database& db, private_message_plugin* plugin)
            : evaluator_impl<private_message_evaluator, private_message_plugin_operation>(db),
              plugin_(plugin)
        {}

        void do_apply(const private_message_operation& o);

        private_message_plugin* plugin_;
    };

    class private_settings_evaluator:
        public evaluator_impl<private_settings_evaluator, private_message_plugin_operation>
    {
    public:
        using operation_type = private_list_operation;

        private_settings_evaluator(database& db, private_message_plugin* plugin)
            : evaluator_impl<private_settings_evaluator, private_message_plugin_operation>(db),
              plugin_(plugin)
        {}

        void do_apply(const private_list_operation& o);

        private_message_plugin* plugin_;
    };

    class private_list_evaluator:
        public evaluator_impl<private_list_evaluator, private_message_plugin_operation>
    {
    public:
        using operation_type = private_list_operation;

        private_list_evaluator(database& db, private_message_plugin* plugin)
            : evaluator_impl<private_list_evaluator, private_message_plugin_operation>(db),
              plugin_(plugin)
        {}

        void do_apply(const private_list_operation& o);

        private_message_plugin* plugin_;
    };


} } }
