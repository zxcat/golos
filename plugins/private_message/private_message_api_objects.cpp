#include <golos/plugins/private_message/private_message_api_objects.hpp>
#include <golos/plugins/private_message/private_message_objects.hpp>

namespace golos { namespace plugins { namespace private_message {

    message_api_object::message_api_object(const message_object& o)
        : from(o.from),
          to(o.to),
          nonce(o.nonce),
          from_memo_key(o.from_memo_key),
          to_memo_key(o.to_memo_key),
          receive_time(o.receive_time),
          checksum(o.checksum),
          read_time(o.read_time),
          encrypted_message(o.encrypted_message.begin(), o.encrypted_message.end()) {
    }

    message_api_object::message_api_object() = default;


    settings_api_object::settings_api_object(const settings_object& o)
        : ignore_messages_from_undefined_contact(o.ignore_messages_from_undefined_contact) {
    }

    settings_api_object::settings_api_object() = default;


    contact_api_object::contact_api_object(const contact_object& o)
        : contact(o.contact),
          json_metadata(o.json_metadata.begin(), o.json_metadata.end()),
          local_type(o.type),
          size(o.size) {
    }

    contact_api_object::contact_api_object() = default;

} } } // golos::plugins::private_message


