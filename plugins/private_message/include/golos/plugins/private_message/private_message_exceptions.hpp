#pragma once

#include <golos/protocol/exceptions.hpp>
#include <golos/plugins/private_message/private_message_plugin.hpp>

#define PLUGIN_CHECK_LOGIC(expr, type, msg, ...) \
    GOLOS_CHECK_LOGIC(expr, type, msg, ("plugin", "private_message") __VA_ARGS__)

namespace golos { namespace plugins { namespace private_message {

    struct logic_errors {
        enum types {
            cannot_send_to_yourself,
            from_and_to_memo_keys_must_be_different,
            cannot_add_contact_to_yourself,
            sender_in_ignore_list,
            recepient_ignore_messages_from_undefined_contact,
            add_undefined_contact,
            contact_has_same_type,
        };
    };

} } } // golos::plugins::private_message

namespace golos {
    template<>
    inline std::string get_logic_error_namespace<golos::plugins::private_message::logic_errors::types>() {
        return golos::plugins::private_message::private_message_plugin::name();
    }
}

FC_REFLECT_ENUM(golos::plugins::private_message::logic_errors::types,
    (cannot_send_to_yourself)
    (from_and_to_memo_keys_must_be_different)
    (cannot_add_contact_to_yourself)
    (sender_in_ignore_list)
    (recepient_ignore_messages_from_undefined_contact)
    (add_undefined_contact)
    (contact_has_same_type)
);