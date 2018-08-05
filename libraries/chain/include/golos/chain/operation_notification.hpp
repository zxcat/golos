#pragma once

#include <golos/protocol/operations.hpp>
#include <golos/chain/steem_object_types.hpp>

namespace golos { namespace chain {

using protocol::operation;

struct operation_notification {
    operation_notification(const operation &o) : op(o) {
    }

    bool stored_in_db = false;
    int64_t db_id = 0;
    transaction_id_type trx_id;
    uint32_t block = 0;
    uint32_t trx_in_block = 0;
    uint16_t op_in_trx = 0;
    uint32_t virtual_op = 0;
    time_point_sec timestamp = time_point_sec(0);
    const operation& op;
};

} } // golos::chain
