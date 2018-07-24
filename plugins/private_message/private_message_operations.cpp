#include <golos/plugins/private_message/private_message_operations.hpp>
#include <golos/protocol/operation_util_impl.hpp>
#include <golos/protocol/exceptions.hpp>
#include <golos/protocol/validate_helper.hpp>
#include <fc/io/json.hpp>

namespace golos { namespace plugins { namespace private_message {

    static inline void validate_account_name(const string &name) {
        GOLOS_CHECK_VALUE(is_valid_account_name(name), "Account name ${name} is invalid", ("name", name));
    }

    static inline bool is_valid_contact_type(private_contact_type type) {
        switch(type) {
            case undefined:
            case pinned:
            case ignored:
                return true;

            default:
                break;
        }
        return false;
    }

    void private_settings_operation::validate() const {
        GOLOS_CHECK_PARAM_ACCOUNT(owner);
    }
    void private_settings_operation::get_required_posting_authorities(flat_set<account_name_type>& a) const {
        a.insert(owner);
    }


    void private_message_operation::validate() const {
        GOLOS_CHECK_PARAM_ACCOUNT(to);

        GOLOS_CHECK_PARAM(from, {
            validate_account_name(from);
            GOLOS_CHECK_VALUE(from != to, "You cannot write to yourself");
        });

        GOLOS_CHECK_PARAM(to_memo_key, {
            GOLOS_CHECK_VALUE(to_memo_key != public_key_type(), "To_key can't be empty");
        });

        GOLOS_CHECK_PARAM(from_memo_key, {
            GOLOS_CHECK_VALUE(from_memo_key != public_key_type(), "From_key can't be empty");
            GOLOS_CHECK_VALUE(from_memo_key != to_memo_key, "From_key can't be equal to to_key");
        });

        GOLOS_CHECK_PARAM(nonce, {
            GOLOS_CHECK_VALUE(nonce != 0, "Nonce can't be zero");
        });

        GOLOS_CHECK_PARAM(encrypted_message, {
            GOLOS_CHECK_VALUE(encrypted_message.size() >= 16, "Encrypted message is too small");
        });
    }


    void private_message_operation::get_required_posting_authorities(flat_set<account_name_type>& a) const {
        a.insert(from);
    }


    void private_contact_operation::validate() const {
        GOLOS_CHECK_PARAM_ACCOUNT(contact);

        GOLOS_CHECK_PARAM(owner, {
            validate_account_name(owner);
            GOLOS_CHECK_VALUE(owner != contact, "You cannot add contact to yourself");
        });

        GOLOS_CHECK_PARAM(type, {
            GOLOS_CHECK_VALUE(is_valid_contact_type(type), "Unknown contact type");
        });

        if (json_metadata.size() > 0) {
            GOLOS_CHECK_PARAM(json_metadata, {
                GOLOS_CHECK_VALUE(fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON");
                GOLOS_CHECK_VALUE(type != undefined, "JSON Metadata can't be set for undefined contact");
            });
        }
    }

    void private_contact_operation::get_required_posting_authorities(flat_set<account_name_type>& a) const {
        a.insert(owner);
    }

} } } // golos::plugins::private_message

DEFINE_OPERATION_TYPE(golos::plugins::private_message::private_message_plugin_operation);