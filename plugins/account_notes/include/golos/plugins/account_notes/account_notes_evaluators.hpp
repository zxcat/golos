#pragma once

#include <golos/plugins/account_notes/account_notes_operations.hpp>
#include <golos/plugins/account_notes/account_notes_plugin.hpp>
#include <golos/chain/database.hpp>
#include <golos/chain/evaluator.hpp>

namespace golos { namespace plugins { namespace account_notes {

using golos::chain::database;
using golos::chain::evaluator;
using golos::chain::evaluator_impl;

class set_value_evaluator : public golos::chain::evaluator_impl<set_value_evaluator, account_notes_plugin_operation> {
public:
    using operation_type = set_value_operation;

    set_value_evaluator(database& db, account_notes_plugin* plugin, const account_notes_settings_api_object* settings)
            : evaluator_impl<set_value_evaluator, account_notes_plugin_operation>(db), plugin_(plugin), settings_(settings) {
    }

    void do_apply(const set_value_operation& o);

private:
    bool is_tracked_account(const account_name_type& account);

    account_notes_plugin* plugin_;

    const account_notes_settings_api_object* settings_;
};

} } } // golos::plugins::account_notes
