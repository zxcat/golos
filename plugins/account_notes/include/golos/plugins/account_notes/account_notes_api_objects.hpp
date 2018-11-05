#pragma once

namespace golos { namespace plugins { namespace account_notes {

struct account_notes_settings_api_object {
    account_notes_settings_api_object() = default;

    account_notes_settings_api_object(uint16_t max_kl, uint16_t max_vl, uint16_t max_nc)
            : max_key_length(max_kl), max_value_length(max_vl), max_note_count(max_nc) {
    }

    uint16_t max_key_length = 0;
    uint16_t max_value_length = 0;
    uint16_t max_note_count = 0;
    flat_set<std::string> tracked_accounts;
    flat_set<std::string> untracked_accounts;
};

} } } // golos::plugins::account_notes

FC_REFLECT((golos::plugins::account_notes::account_notes_settings_api_object),
     (max_key_length)(max_value_length)(max_note_count)(tracked_accounts)(untracked_accounts));
