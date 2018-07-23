#pragma once

#include <golos/protocol/base.hpp>
#include <golos/protocol/types.hpp>
#include <golos/chain/steem_object_types.hpp>
#include <chainbase/chainbase.hpp>
#include <golos/protocol/operation_util.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace golos { namespace plugins { namespace private_message {

    using namespace golos::protocol;
    using namespace chainbase;
    using namespace golos::chain;
    using namespace boost::multi_index;

#ifndef PRIVATE_MESSAGE_SPACE_ID
#define PRIVATE_MESSAGE_SPACE_ID 6
#endif

    enum private_message_object_type {
        message_object_type = (PRIVATE_MESSAGE_SPACE_ID << 8),
        list_object_type = (PRIVATE_MESSAGE_SPACE_ID << 8) + 1,
        list_size_object_type = (PRIVATE_MESSAGE_SPACE_ID << 8) + 2,
    };

    class message_object: public object<message_object_type, message_object> {
    public:
        template<typename Constructor, typename Allocator>
        message_object(Constructor&& c, allocator <Allocator> a)
            : encrypted_message(a) {
            c(*this);
        }

        id_type id;

        account_name_type from;
        account_name_type to;
        public_key_type from_memo_key;
        public_key_type to_memo_key;
        uint64_t sent_time = 0; /// used as seed to secret generation
        time_point_sec receive_time; /// time received by blockchain
        uint32_t checksum = 0;
        time_point_sec read_time;
        buffer_type encrypted_message;
    };

    using message_id_type = message_object::id_type;

    struct message_api_object {
        message_api_object(const message_object& o);
        message_api_object();

        account_name_type from;
        account_name_type to;
        public_key_type from_memo_key;
        public_key_type to_memo_key;
        uint64_t sent_time = 0;
        time_point_sec receive_time;
        uint32_t checksum = 0;
        time_point_sec read_time;
        std::vector<char> encrypted_message;
    };

    struct by_to_date;
    struct by_from_date;

    using message_index = multi_index_container<
        message_object,
        indexed_by<
            ordered_unique<
                tag<by_id>,
                member<message_object, message_id_type, &message_object::id>>,
            ordered_unique<
                tag<by_to_date>,
                composite_key<
                    message_object,
                    member<message_object, account_name_type, &message_object::to>,
                    member<message_object, time_point_sec, &message_object::receive_time>,
                    member<message_object, message_id_type, &message_object::id>>,
                composite_key_compare<
                    string_less,
                    std::greater<time_point_sec>,
                    std::less<message_id_type>>>,
            ordered_unique<
                tag<by_from_date>,
                composite_key<
                    message_object,
                    member<message_object, account_name_type, &message_object::from>,
                    member<message_object, time_point_sec, &message_object::receive_time>,
                    member<message_object, message_id_type, &message_object::id>>,
                composite_key_compare<
                    string_less,
                    std::greater<time_point_sec>,
                    std::less<message_id_type>>>>,
        allocator<message_object>>;

    struct private_message_operation: public base_operation {
        account_name_type from;
        account_name_type to;
        public_key_type from_memo_key;
        public_key_type to_memo_key;
        uint64_t sent_time = 0; /// used as seed to secret generation
        uint32_t checksum = 0;
        std::vector<char> encrypted_message;

        void validate() const;
        void get_required_posting_authorities(flat_set<account_name_type>& a) const;
    };

    enum private_list_type: uint8_t {
        undefined = 1,
        pinned = 2,
        ignored = 3,
    };

    constexpr auto private_list_type_size = ignored + 1;

    class list_object: public object<list_object_type, list_object> {
    public:
        template<typename Constructor, typename Allocator>
        list_object(Constructor&& c, allocator <Allocator> a) {
            c(*this);
        }

        id_type id;

        account_name_type owner;
        account_name_type contact;
        private_list_type type;
        uint32_t total_send_messages = 0;
        uint32_t unread_send_messages = 0;
        uint32_t total_recv_messages = 0;
        uint32_t unread_recv_messages = 0;
    };

    using list_id_type = list_object::id_type;

    struct list_api_object {
        list_api_object(const list_object& o);
        list_api_object();

        account_name_type owner;
        account_name_type contact;
        private_list_type owner_type = undefined;
        private_list_type contact_type = undefined;
        uint32_t total_send_messages = 0;
        uint32_t unread_send_messages = 0;
        uint32_t total_recv_messages = 0;
        uint32_t unread_recv_messages = 0;
    };

    struct by_owner;
    struct by_contact;

    using list_index = multi_index_container<
        list_object,
        indexed_by<
            ordered_unique<
                tag<by_id>,
                member<list_object, list_id_type, &list_object::id>>,
            ordered_unique<
                tag<by_owner>,
                composite_key<
                    list_object,
                    member<list_object, account_name_type, &list_object::owner>,
                    member<list_object, private_list_type, &list_object::type>,
                    member<list_object, account_name_type, &list_object::contact>>,
                composite_key_compare<
                    string_less,
                    std::greater<private_list_type>,
                    string_less>>,
            ordered_unique<
                tag<by_contact>,
                composite_key<
                    list_object,
                    member<list_object, account_name_type, &list_object::owner>,
                    member<list_object, account_name_type, &list_object::contact>>,
                composite_key_compare<
                    string_less,
                    string_less>>>,
        allocator<list_object>>;

    struct list_size_info {
        uint32_t total_contacts = 0;
        uint32_t total_send_messages = 0;
        uint32_t unread_send_messages = 0;
        uint32_t total_recv_messages = 0;
        uint32_t unread_recv_messages = 0;
    };

    struct list_size_object: public object<list_size_object_type, list_size_object> {
        template<typename Constructor, typename Allocator>
        list_size_object(Constructor&& c, allocator <Allocator> a) {
            c(*this);
        }

        id_type id;

        account_name_type owner;
        private_list_type type;
        list_size_info info;
    };

    using list_size_id_type = list_size_object::id_type;

    struct list_size_api_object {
        account_name_type owner;
        fc::flat_map<private_list_type, list_size_info> info;
    };

    using list_size_index = multi_index_container<
        list_size_object,
        indexed_by<
            ordered_unique<
                tag<by_id>,
                member<list_size_object, list_size_id_type, &list_size_object::id>>,
            ordered_unique<
                tag<by_owner>,
                composite_key<
                    list_size_object,
                    member<list_size_object, account_name_type, &list_size_object::owner>,
                    member<list_size_object, private_list_type, &list_size_object::type>>,
                composite_key_compare<
                    string_less,
                    std::less<private_list_type>>>>,
        allocator<list_size_object>>;

    struct private_list_operation: public base_operation {
        account_name_type owner;
        account_name_type contact;
        private_list_type type;

        void validate() const;
        void get_required_posting_authorities(flat_set<account_name_type>& a) const;
    };

    using private_message_plugin_operation = fc::static_variant<
        private_message_operation,
        private_list_operation>;

} } } // golos::plugins::private_message

CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::private_message::message_object, golos::plugins::private_message::message_index)

CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::private_message::list_object, golos::plugins::private_message::list_index)

CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::private_message::list_size_object, golos::plugins::private_message::list_size_index)

FC_REFLECT(
    (golos::plugins::private_message::message_api_object),
    (from)(to)(from_memo_key)(to_memo_key)(sent_time)(receive_time)(read_time)(checksum)(encrypted_message))

FC_REFLECT_ENUM(
    golos::plugins::private_message::private_list_type,
    (undefined)(pinned)(ignored))

FC_REFLECT(
    (golos::plugins::private_message::list_size_info),
    (total_contacts)(total_send_messages)(unread_send_messages)(total_recv_messages)(unread_recv_messages))

FC_REFLECT(
    (golos::plugins::private_message::list_api_object),
    (owner)(contact)(owner_type)(contact_type)
    (total_send_messages)(unread_send_messages)(total_recv_messages)(unread_recv_messages))

FC_REFLECT(
    (golos::plugins::private_message::list_size_api_object),
    (owner)(info))

FC_REFLECT(
    (golos::plugins::private_message::private_message_operation),
    (from)(to)(from_memo_key)(to_memo_key)(sent_time)(checksum)(encrypted_message))

FC_REFLECT(
    (golos::plugins::private_message::private_list_operation),
    (owner)(contact)(type))

FC_REFLECT_TYPENAME((golos::plugins::private_message::private_message_plugin_operation))

DECLARE_OPERATION_TYPE(golos::plugins::private_message::private_message_plugin_operation)