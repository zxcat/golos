#pragma once

#include <golos/protocol/base.hpp>
#include <golos/chain/evaluator.hpp>

namespace golos { namespace plugins { namespace account_notes {

using golos::protocol::base_operation;
using golos::protocol::account_name_type;

struct set_value_operation : base_operation {
    account_name_type account;
    string key;
    string value;

    void validate() const;

    void get_required_active_authorities(flat_set<account_name_type>& a) const {
        a.insert(account);
    }
};

using account_notes_plugin_operation = fc::static_variant<set_value_operation>;

} } } // golos::plugins::account_notes

FC_REFLECT((golos::plugins::account_notes::set_value_operation), (account)(key)(value));

FC_REFLECT_TYPENAME((golos::plugins::account_notes::account_notes_plugin_operation));
DECLARE_OPERATION_TYPE(golos::plugins::account_notes::account_notes_plugin_operation)
