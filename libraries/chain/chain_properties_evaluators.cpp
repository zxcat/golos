#include <golos/chain/steem_evaluator.hpp>
#include <golos/chain/database.hpp>
#include <golos/chain/steem_objects.hpp>
#include <golos/protocol/validate_helper.hpp>

namespace golos { namespace chain {

    void witness_update_evaluator::do_apply(const witness_update_operation& o) {
        _db.get_account(o.owner); // verify owner exists

        if (_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
            GOLOS_CHECK_OP_PARAM(o, url, {
                GOLOS_CHECK_VALUE_MAX_SIZE(o.url, STEEMIT_MAX_WITNESS_URL_LENGTH);
            });
        } else if (o.url.size() > STEEMIT_MAX_WITNESS_URL_LENGTH) {
            // after HF, above check can be moved to validate() if reindex doesn't show this warning
            wlog("URL is too long in block ${b}", ("b", _db.head_block_num() + 1));
        }

        const bool has_hf18 = _db.has_hardfork(STEEMIT_HARDFORK_0_18__673);

        // TODO: remove this after HF 18
        if (has_hf18) {
            if (o.props.account_creation_fee.amount.value != STEEMIT_MIN_ACCOUNT_CREATION_FEE) {
                wlog("The chain_properties_update_operation should be used to update account_creation_fee");
            }
            if (o.props.sbd_interest_rate != STEEMIT_DEFAULT_SBD_INTEREST_RATE) {
                wlog("The chain_properties_update_operation should be used to update sbd_interest_rate");
            }
            if (o.props.maximum_block_size != STEEMIT_MIN_BLOCK_SIZE_LIMIT * 2) {
                wlog("The chain_properties_update_operation should be used to update maximum_block_size");
            }
        }

        auto update_witness = [&](witness_object& w) {
            from_string(w.url, o.url);
            w.signing_key = o.block_signing_key;
            if (!has_hf18) {
                w.props = o.props;
            }
        };

        const auto& idx = _db.get_index<witness_index>().indices().get<by_name>();
        auto itr = idx.find(o.owner);
        if (itr != idx.end()) {
            _db.modify(*itr, update_witness);
        } else {
            _db.create<witness_object>([&](witness_object& w) {
                w.owner = o.owner;
                w.created = _db.head_block_time();
                update_witness(w);
            });
        }
    }

    struct chain_properties_convert {
        using result_type = chain_properties_18;

        template<typename Props>
        result_type operator()(Props&& p) const {
            result_type r;
            r = p;
            return r;
        }
    };

    void chain_properties_update_evaluator::do_apply(const chain_properties_update_operation& o) {
        _db.get_account(o.owner); // verify owner exists

        const auto& idx = _db.get_index<witness_index>().indices().get<by_name>();
        auto itr = idx.find(o.owner);
        if (itr != idx.end()) {
            _db.modify(*itr, [&](witness_object& w) {
                w.props = o.props.visit(chain_properties_convert());
            });
        } else {
            _db.create<witness_object>([&](witness_object& w) {
                w.owner = o.owner;
                w.created = _db.head_block_time();
                w.props = o.props.visit(chain_properties_convert());
            });
        }
    }

} } // golos::chain
