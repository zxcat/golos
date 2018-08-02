#pragma once

#include <golos/protocol/base.hpp>

#include <golos/plugins/private_message/private_message_operations.hpp>

#define PRIVATE_DEFAULT_LIMIT 100

namespace golos { namespace plugins { namespace private_message {

    using namespace golos::protocol;

    class message_object;

    struct message_api_object {
        message_api_object(const message_object& o);
        message_api_object();

        account_name_type from;
        account_name_type to;
        uint64_t nonce = 0;
        public_key_type from_memo_key;
        public_key_type to_memo_key;
        uint32_t checksum = 0;
        std::vector<char> encrypted_message;

        time_point_sec create_date;
        time_point_sec receive_date;
        time_point_sec read_date;
        time_point_sec remove_date;
    };

    class settings_object;

    struct settings_api_object {
        settings_api_object(const settings_object& o);
        settings_api_object();

        bool ignore_messages_from_unknown_contact = false;
    };

    struct contact_size_info {
        uint32_t total_outbox_messages = 0;
        uint32_t unread_outbox_messages = 0;
        uint32_t total_inbox_messages = 0;
        uint32_t unread_inbox_messages = 0;

        bool empty() const {
            return !total_outbox_messages && !total_inbox_messages;
        }

        contact_size_info& operator-=(const contact_size_info& s) {
            total_outbox_messages -= s.total_outbox_messages;
            unread_outbox_messages -= s.unread_outbox_messages;
            total_inbox_messages -= s.total_inbox_messages;
            unread_inbox_messages -= s.unread_inbox_messages;
            return *this;
        }

        contact_size_info& operator+=(const contact_size_info& s) {
            total_outbox_messages += s.total_outbox_messages;
            unread_outbox_messages += s.unread_outbox_messages;
            total_inbox_messages += s.total_inbox_messages;
            unread_inbox_messages += s.unread_inbox_messages;
            return *this;
        }

        bool operator==(const contact_size_info& s) const {
            return
                total_outbox_messages == s.total_outbox_messages &&
                unread_outbox_messages == s.unread_outbox_messages &&
                total_inbox_messages == s.total_inbox_messages &&
                unread_inbox_messages == s.unread_inbox_messages;
        }

        bool operator!=(const contact_size_info& s) const {
            return !(this->operator==(s));
        }
    };

    struct contacts_size_info final: public contact_size_info {
        uint32_t total_contacts = 0;
    };

    /**
     * Contact item
     */
    class contact_object;

    struct contact_api_object {
        contact_api_object(const contact_object& o);
        contact_api_object();

        account_name_type owner;
        account_name_type contact;
        std::string json_metadata;
        private_contact_type local_type = unknown;
        private_contact_type remote_type = unknown;
        contact_size_info size;
    };

    /**
     * Counters for account contact lists
     */
    struct contact_size_object;

    struct contacts_size_api_object {
        fc::flat_map<private_contact_type, contacts_size_info> size;
    };

    /**
     * Query for inbox/outbox messages
     */
    struct message_box_query {
        fc::flat_set<std::string> select_accounts;
        fc::flat_set<std::string> filter_accounts;
        time_point_sec newest_date = time_point_sec::min();
        bool unread_only = false;
        uint16_t limit = PRIVATE_DEFAULT_LIMIT;
        uint32_t offset = 0;
    };
    
    /**
     * Query for thread messages
     */
    struct message_thread_query {
        time_point_sec newest_date = time_point_sec::min();
        bool unread_only = false;
        uint16_t limit = PRIVATE_DEFAULT_LIMIT;
        uint32_t offset = 0;
    };
    
} } } // golos::plugins::private_message

FC_REFLECT(
    (golos::plugins::private_message::message_api_object),
    (from)(to)(from_memo_key)(to_memo_key)(nonce)(checksum)(encrypted_message)
    (create_date)(receive_date)(read_date)(remove_date))

FC_REFLECT(
    (golos::plugins::private_message::settings_api_object),
    (ignore_messages_from_unknown_contact))

FC_REFLECT(
    (golos::plugins::private_message::contact_size_info),
    (total_outbox_messages)(unread_outbox_messages)(total_inbox_messages)(unread_inbox_messages))

FC_REFLECT_DERIVED(
    (golos::plugins::private_message::contacts_size_info), ((golos::plugins::private_message::contact_size_info)),
    (total_contacts))

FC_REFLECT(
    (golos::plugins::private_message::contact_api_object),
    (contact)(json_metadata)(local_type)(remote_type)(size))

FC_REFLECT(
    (golos::plugins::private_message::contacts_size_api_object),
    (size))

FC_REFLECT(
    (golos::plugins::private_message::message_box_query),
    (select_accounts)(filter_accounts)(newest_date)(unread_only)(limit)(offset))

FC_REFLECT(
    (golos::plugins::private_message::message_thread_query),
    (newest_date)(unread_only)(limit)(offset))