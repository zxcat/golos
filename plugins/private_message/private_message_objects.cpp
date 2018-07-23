#include <golos/plugins/private_message/private_message_objects.hpp>
#include <golos/protocol/operation_util_impl.hpp>
#include <golos/protocol/exceptions.hpp>
#include <golos/protocol/validate_helper.hpp>

namespace golos { namespace plugins { namespace private_message {

    static inline void validate_account_name(const string &name) {
        GOLOS_CHECK_VALUE(is_valid_account_name(name), "Account name ${name} is invalid", ("name", name));
    }

    message_api_object::message_api_object(const message_object& o)
        : from(o.from),
          to(o.to),
          from_memo_key(o.from_memo_key),
          to_memo_key(o.to_memo_key),
          sent_time(o.sent_time),
          receive_time(o.receive_time),
          checksum(o.checksum),
          read_time(o.read_time),
          encrypted_message(o.encrypted_message.begin(), o.encrypted_message.end()) {
    }

    message_api_object::message_api_object() = default;

    list_api_object::list_api_object(const list_object& o)
        : owner(o.owner),
          contact(o.contact),
          owner_type(o.type),
          total_send_messages(o.total_send_messages),
          unread_send_messages(o.unread_send_messages),
          total_recv_messages(o.total_recv_messages),
          unread_recv_messages(o.unread_recv_messages) {
    }

    list_api_object::list_api_object() = default;

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

        GOLOS_CHECK_PARAM(sent_time, {
            GOLOS_CHECK_VALUE(sent_time != 0, "Send time can't be zero");
        });

        GOLOS_CHECK_PARAM(encrypted_message, {
            GOLOS_CHECK_VALUE(encrypted_message.size() >= 16, "Encrypted message is too small");
        });
    }

    void private_message_operation::get_required_posting_authorities(flat_set<account_name_type>& a) const {
        a.insert(from);
    }

    bool is_valid_list_type(private_list_type type) {
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

    void private_list_operation::validate() const {
        GOLOS_CHECK_PARAM_ACCOUNT(contact);

        GOLOS_CHECK_PARAM(owner, {
            validate_account_name(owner);
            GOLOS_CHECK_VALUE(owner != contact, "You cannot add contact to yourself");
        });

        GOLOS_CHECK_PARAM(type, {
            GOLOS_CHECK_VALUE(is_valid_list_type(type), "Unknown list type");
        });
    }

    void private_list_operation::get_required_posting_authorities(flat_set<account_name_type>& a) const {
        a.insert(owner);
    }

} } } // golos::plugins::private_message

DEFINE_OPERATION_TYPE(golos::plugins::private_message::private_message_plugin_operation);
