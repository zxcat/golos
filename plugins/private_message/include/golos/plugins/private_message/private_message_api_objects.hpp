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
        time_point_sec create_time;
        time_point_sec receive_time;
        uint32_t checksum = 0;
        time_point_sec read_time;
        std::vector<char> encrypted_message;
    };

    class settings_object;

    struct settings_api_object {
        settings_api_object(const settings_object& o);
        settings_api_object();

        bool ignore_messages_from_undefined_contact = false;
    };

    struct contact_size_info {
        uint32_t total_send_messages = 0;
        uint32_t unread_send_messages = 0;
        uint32_t total_recv_messages = 0;
        uint32_t unread_recv_messages = 0;

        bool empty() const {
            return !total_send_messages && !total_recv_messages;
        }

        contact_size_info& operator-=(const contact_size_info& s) {
            total_send_messages -= s.total_send_messages;
            unread_send_messages -= s.unread_send_messages;
            total_recv_messages -= s.total_recv_messages;
            unread_recv_messages -= s.unread_send_messages;
            return *this;
        }

        contact_size_info& operator+=(const contact_size_info& s) {
            total_send_messages += s.total_send_messages;
            unread_send_messages += s.unread_send_messages;
            total_recv_messages += s.total_recv_messages;
            unread_recv_messages += s.unread_send_messages;
            return *this;
        }

        bool operator==(const contact_size_info& s) const {
            return
                total_send_messages == s.total_send_messages &&
                unread_send_messages == s.unread_send_messages &&
                total_recv_messages == s.total_recv_messages &&
                unread_recv_messages == s.unread_recv_messages;
        }

        bool operator!=(const contact_size_info& s) const {
            return !(this->operator==(s));
        }
    };

    struct contacts_size_info final: public contact_size_info {
        uint32_t total_contacts;
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
        private_contact_type local_type = undefined;
        private_contact_type remote_type = undefined;
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
     * Query for inbox messages
     */
    struct inbox_query {
        fc::flat_set<std::string> select_from;
        time_point_sec start_date = time_point_sec::min();
        bool unread_only = false;
        uint16_t limit = PRIVATE_DEFAULT_LIMIT;
        uint32_t offset = 0;
    };

    /**
     * Query for outbox messages
     */
    struct outbox_query {
        fc::flat_set<std::string> select_to;
        time_point_sec start_date = time_point_sec::min();
        bool unread_only = false;
        uint16_t limit = PRIVATE_DEFAULT_LIMIT;
        uint32_t offset = 0;
    };

} } } // golos::plugins::private_message

FC_REFLECT(
    (golos::plugins::private_message::message_api_object),
    (from)(to)(from_memo_key)(to_memo_key)(nonce)
    (create_time)(receive_time)(read_time)(checksum)(encrypted_message))

FC_REFLECT(
    (golos::plugins::private_message::settings_api_object),
    (ignore_messages_from_undefined_contact))

FC_REFLECT(
    (golos::plugins::private_message::contact_size_info),
    (total_send_messages)(unread_send_messages)(total_recv_messages)(unread_recv_messages))

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
    (golos::plugins::private_message::inbox_query),
    (select_from)(start_date)(unread_only)(limit)(offset))

FC_REFLECT(
    (golos::plugins::private_message::outbox_query),
    (select_to)(start_date)(unread_only)(limit)(offset))