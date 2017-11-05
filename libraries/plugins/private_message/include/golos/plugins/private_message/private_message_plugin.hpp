#pragma once


#include <golos/plugins/chain/plugin.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <appbase/application.hpp>

namespace golos {
    namespace plugins {
        namespace private_message {
            using namespace golos::chain;

            //
            // Plugins should #define their SPACE_ID's so plugins with
            // conflicting SPACE_ID assignments can be compiled into the
            // same binary (by simply re-assigning some of the conflicting #defined
            // SPACE_ID's in a build script).
            //
            // Assignment of SPACE_ID's cannot be done at run-time because
            // various template automagic depends on them being known at compile
            // time.
            //
#ifndef PRIVATE_MESSAGE_SPACE_ID
#define PRIVATE_MESSAGE_SPACE_ID 6
#endif

#define STEEMIT_PRIVATE_MESSAGE_COP_ID 777

            enum private_message_object_type {
                message_object_type = (PRIVATE_MESSAGE_SPACE_ID << 8)
            };


            struct message_body {
                fc::time_point thread_start; /// the sent_time of the original message, if any
                string subject;
                string body;
                string json_meta;
                flat_set<string> cc;
            };


            class message_object : public object<message_object_type, message_object> {
            public:
                template<typename Constructor, typename Allocator>
                message_object(Constructor &&c, allocator<Allocator> a) :
                        encrypted_message(a) {
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
                buffer_type encrypted_message;
            };

            using message_id_type = message_object::id_type;

            struct message_api_obj {
                message_api_obj(const message_object &o) : id(o.id), from(o.from), to(o.to),
                        from_memo_key(o.from_memo_key), to_memo_key(o.to_memo_key), sent_time(o.sent_time),
                        receive_time(o.receive_time), checksum(o.checksum),
                        encrypted_message(o.encrypted_message.begin(), o.encrypted_message.end()) {
                }

                message_api_obj() {
                }

                message_id_type id;
                account_name_type from;
                account_name_type to;
                public_key_type from_memo_key;
                public_key_type to_memo_key;
                uint64_t sent_time;
                time_point_sec receive_time;
                uint32_t checksum;
                vector<char> encrypted_message;
            };

            struct extended_message_object : public message_api_obj {
                extended_message_object() {
                }

                extended_message_object(const message_api_obj &o) : message_api_obj(o) {
                }

                message_body message;
            };

            struct by_to_date;
            struct by_from_date;

            using namespace boost::multi_index;

            typedef multi_index_container<message_object,
                    indexed_by<ordered_unique<tag<by_id>, member<message_object, message_id_type, &message_object::id>>,
                            ordered_unique<tag<by_to_date>, composite_key<message_object,
                                    member<message_object, account_name_type, &message_object::to>,
                                    member<message_object, time_point_sec, &message_object::receive_time>,
                                    member<message_object, message_id_type, &message_object::id> >,
                                    composite_key_compare<std::less<string>, std::greater<time_point_sec>,
                                            std::less<message_id_type>>>, ordered_unique<tag<by_from_date>,
                                    composite_key<message_object,
                                            member<message_object, account_name_type, &message_object::from>,
                                            member<message_object, time_point_sec, &message_object::receive_time>,
                                            member<message_object, message_id_type, &message_object::id> >,
                                    composite_key_compare<std::less<string>, std::greater<time_point_sec>,
                                            std::less<message_id_type>>> >, allocator<message_object> > message_index;


            /**
             *   This plugin scans the blockchain for custom operations containing a valid message and authorized
             *   by the posting key.
             *
             */
            class private_message_plugin final : public appbase::plugin<private_message_plugin> {
            public:
                constexpr static const char *plugin_name = "private_message";

                static const std::string &name() {
                    static std::string name = plugin_name;
                    return name;
                }

                APPBASE_PLUGIN_REQUIRES((chain::plugin))

                private_message_plugin();

                ~private_message_plugin();

                void set_program_options(boost::program_options::options_description &cli,
                                         boost::program_options::options_description &cfg) override;

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override;

                void plugin_shutdown() override {
                }

                flat_map<string, string> tracked_accounts() const; /// map start_range to end_range
            private:
                struct private_message_plugin_impl;

                std::unique_ptr<private_message_plugin_impl> my;
            };


        }
    }
} //golos::private_message


FC_REFLECT((golos::plugins::private_message::message_body), (thread_start)(subject)(body)(json_meta)(cc));

FC_REFLECT((golos::plugins::private_message::message_object),
           (id)(from)(to)(from_memo_key)(to_memo_key)(sent_time)(receive_time)(checksum)(encrypted_message));
CHAINBASE_SET_INDEX_TYPE(golos::plugins::private_message::message_object,
                         golos::plugins::private_message::message_index);

FC_REFLECT((golos::plugins::private_message::message_api_obj),
           (id)(from)(to)(from_memo_key)(to_memo_key)(sent_time)(receive_time)(checksum)(encrypted_message));

FC_REFLECT_DERIVED((golos::plugins::private_message::extended_message_object),
                   ((golos::plugins::private_message::message_api_obj)), (message));
