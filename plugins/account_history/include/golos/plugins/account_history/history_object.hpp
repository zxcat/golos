#pragma once

#include <golos/protocol/authority.hpp>
#include <golos/protocol/operations.hpp>
#include <golos/protocol/steem_operations.hpp>

#include <chainbase/chainbase.hpp>

#include <golos/chain/index.hpp>
#include <golos/chain/steem_object_types.hpp>

#include <golos/plugins/operation_history/history_object.hpp>

#include <boost/multi_index/composite_key.hpp>

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

#ifndef ACCOUNT_HISTORY_SPACE_ID
#define ACCOUNT_HISTORY_SPACE_ID 12
#endif


namespace golos { namespace plugins { namespace account_history {

    enum account_object_types {
        account_history_object_type = (ACCOUNT_HISTORY_SPACE_ID << 8)
    };

    enum operation_direction : uint8_t {
        any = 0,
        sender,
        receiver,
        dual,
    };

    struct account_history_query final {
        fc::optional<fc::flat_set<std::string>> select_ops;
        fc::optional<fc::flat_set<std::string>> filter_ops;
        fc::optional<operation_direction> direction;
    };

    using namespace golos::chain;
    using namespace chainbase;

    template<bool T> struct op_tag_dir;
    // compact version
    template<> struct op_tag_dir<true> {
        uint8_t op_tag:6;
        operation_direction dir:2;
    };
    // normal version
    template<> struct op_tag_dir<false> {
        uint8_t op_tag;
        operation_direction dir;
    };
static_assert(protocol::operation::count() >= 0 && protocol::operation::count() <= 0xFF,
    "There are more ops than u8 type can handle. Please, update op_tag type");

    using golos::plugins::operation_history::operation_id_type;

    class account_history_object final: public object<account_history_object_type, account_history_object> {
    public:
        template <typename Constructor, typename Allocator>
        account_history_object(Constructor&& c, allocator <Allocator> a) {
            c(*this);
        }

        id_type id;

        account_name_type account;
        uint32_t block = 0;
        uint32_t sequence = 0;
        operation_id_type op;

// #ifdef ACC_HISTORY_COMPACT_OBJ
//         struct {
//             uint8_t op_tag:6;
//             operation_direction dir:2;
//         };
// #else
//         uint8_t op_tag;
//         operation_direction dir;
// #endif  //ACC_HISTORY_COMPACT_OBJ
        op_tag_dir<protocol::operation::count() >= 0 && protocol::operation::count() <= 0x3F> opdir;

        uint8_t get_op_tag() const {
            return opdir.op_tag;
        }
        operation_direction get_dir() const {
            return opdir.dir;
        }
        void set_op_tag_dir(uint8_t tag, operation_direction d) {
            opdir.op_tag = tag;
            opdir.dir = d;
        }
    };

    using account_history_id_type = object_id<account_history_object>;

    struct by_location;
    struct by_operation;
    struct by_timestamp;
    struct by_account;
    using account_history_index = multi_index_container<
        account_history_object,
        indexed_by<
            ordered_unique<
                tag<by_id>,
                member<account_history_object, account_history_id_type, &account_history_object::id>>,
            ordered_non_unique<
                tag<by_location>,
                member<account_history_object, uint32_t, &account_history_object::block>>,
            ordered_unique<
                tag<by_operation>,
                composite_key<account_history_object,
                    member<account_history_object, account_name_type, &account_history_object::account>,
                    const_mem_fun<account_history_object, uint8_t, &account_history_object::get_op_tag>,
                    const_mem_fun<account_history_object, operation_direction, &account_history_object::get_dir>,
                    member<account_history_object, uint32_t, &account_history_object::sequence>>,
                composite_key_compare<
                    std::less<account_name_type>, std::less<uint8_t>, std::less<uint8_t>, std::greater<uint32_t>>>,
            // ordered_unique<
            //     tag<by_direction>,
            //     composite_key<account_history_object,
            //         member<account_history_object, account_name_type, &account_history_object::account>,
            //         member<account_history_object, operation_direction, &account_history_object::dir>,
            //         member<account_history_object, uint32_t, &account_history_object::sequence>>,
            //     composite_key_compare<std::less<account_name_type>, std::less<uint8_t>, std::greater<uint32_t>>>,
            ordered_unique<
                tag<by_account>,
                composite_key<account_history_object,
                    member<account_history_object, account_name_type, &account_history_object::account>,
                    member<account_history_object, uint32_t, &account_history_object::sequence>>,
                composite_key_compare<std::less<account_name_type>, std::greater<uint32_t>>>>,
        allocator<account_history_object>>;

} } } // golos::plugins::account_history

FC_REFLECT_ENUM(golos::plugins::account_history::operation_direction, (any)(sender)(receiver)(dual))

FC_REFLECT((golos::plugins::account_history::account_history_query),
    (select_ops)(filter_ops)(direction))

CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::account_history::account_history_object,
    golos::plugins::account_history::account_history_index)

