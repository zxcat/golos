#include <golos/plugins/account_notes/account_notes_operations.hpp>
#include <golos/plugins/account_notes/account_notes_objects.hpp>
#include <golos/plugins/account_notes/account_notes_evaluators.hpp>
#include <golos/chain/account_object.hpp>

namespace golos { namespace plugins { namespace account_notes {

using golos::chain::from_string;

bool set_value_evaluator::is_tracked_account(const account_name_type& account) {
    // Check if account occures in whitelist if whitelist is set in config
    if (!settings_->tracked_accounts.empty()
        && settings_->tracked_accounts.find(std::string(account)) == settings_->tracked_accounts.end()) {
        return false;
    }

    if (settings_->untracked_accounts.find(std::string(account)) != settings_->untracked_accounts.end()) {
        return false;
    }

    return true;
}

void set_value_evaluator::do_apply(const set_value_operation& o) {
    try {
        if (!is_tracked_account(o.account)) {
            return;
        };

        if (o.key.size() < 1 || o.key.size() > settings_->max_key_length) {
            return;
        }

        if (o.value.size() > settings_->max_value_length) {
            return;
        }

        const auto& notes_idx = db().get_index<account_note_index, by_account_key>();
        auto notes_itr = notes_idx.find(boost::make_tuple(o.account, o.key));

        const auto& stats_idx = db().get_index<account_note_stats_index, by_account>();
        auto stats_itr = stats_idx.find(o.account);

        if (o.value.empty()) { // Delete case
            if (notes_itr != notes_idx.end()) {
                db().remove(*notes_itr);

                db().modify(*stats_itr, [&](account_note_stats_object& ns) {
                    ns.note_count--;
                });
            }
            return;
        }

        if (notes_itr != notes_idx.end()) { // Edit case
            db().modify(*notes_itr, [&](account_note_object& n) {
                from_string(n.value, o.value);
            });
        } else { // Create case
            if (stats_itr != stats_idx.end()) {
                if (stats_itr->note_count >= settings_->max_note_count) {
                    return;
                }

                db().modify(*stats_itr, [&](account_note_stats_object& ns) {
                    ns.note_count++;
                });
            } else {
                db().create<account_note_stats_object>([&](account_note_stats_object& n) {
                    n.account = o.account;
                    n.note_count = 1;
                });
            }

            db().create<account_note_object>([&](account_note_object& n) {
                n.account = o.account;
                from_string(n.key, o.key);
                from_string(n.value, o.value);
            });
        }
    }
    FC_CAPTURE_AND_RETHROW((o))
}

} } } // golos::plugins::account_notes
