#include <golos/plugins/private_message/private_message_evaluators.hpp>

namespace golos {
    namespace plugins {
        namespace private_message {
            void private_message_evaluator::do_apply(const private_message_operation &pm) {
                database &d = get_database();

                const flat_map<string, string> &tracked_accounts = _plugin->tracked_accounts();

                auto to_itr = tracked_accounts.lower_bound(pm.to);
                auto from_itr = tracked_accounts.lower_bound(pm.from);

                FC_ASSERT(pm.from != pm.to);
                FC_ASSERT(pm.from_memo_key != pm.to_memo_key);
                FC_ASSERT(pm.sent_time != 0);
                FC_ASSERT(pm.encrypted_message.size() >= 32);

                if (!tracked_accounts.size() ||
                    (to_itr != tracked_accounts.end() && pm.to >= to_itr->first && pm.to <= to_itr->second) ||
                    (from_itr != tracked_accounts.end() && pm.from >= from_itr->first && pm.from <= from_itr->second)) {
                    d.create<message_object>([&](message_object &pmo) {
                        pmo.from = pm.from;
                        pmo.to = pm.to;
                        pmo.from_memo_key = pm.from_memo_key;
                        pmo.to_memo_key = pm.to_memo_key;
                        pmo.checksum = pm.checksum;
                        pmo.sent_time = pm.sent_time;
                        pmo.receive_time = d.head_block_time();
                        pmo.encrypted_message.resize(pm.encrypted_message.size());
                        std::copy(pm.encrypted_message.begin(), pm.encrypted_message.end(),
                                  pmo.encrypted_message.begin());
                    });
                }
            }
        }
    }
}