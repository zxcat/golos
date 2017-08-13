#include <steemit/chain/database_exceptions.hpp>

#include <steemit/protocol/operations/confidential_operations.hpp>
#include <steemit/chain/confidential_evaluator.hpp>
#include <steemit/chain/confidential_object.hpp>
#include <steemit/chain/database.hpp>
#include <steemit/chain/hardfork.hpp>

#include <fc/smart_ref_impl.hpp>

namespace steemit {
    namespace chain {
        void transfer_to_blind_evaluator::do_apply(const transfer_to_blind_operation &o) {
            try {
                const auto &d = db();

                const auto &atype = o.amount.asset_id(db());
                FC_ASSERT(atype.allow_confidential());
                FC_ASSERT(!atype.is_transfer_restricted());
                FC_ASSERT(!(atype.options.flags & white_list));

                for (const auto &out : o.outputs) {
                    for (const auto &a : out.owner.account_auths) {
                        a.first(d);
                    } // verify all accounts exist and are valid
                }
            } FC_CAPTURE_AND_RETHROW((o))

            try {
                db().adjust_balance(o.from, -o.amount);

                const auto &add = o.amount.asset_id(db()).dynamic_asset_data_id(db());  // verify fee is a legit asset
                db().modify(add, [&](asset_dynamic_data_object &obj) {
                    obj.confidential_supply += o.amount.amount;
                    FC_ASSERT(obj.confidential_supply >= 0);
                });
                for (const auto &out : o.outputs) {
                    db().create<blinded_balance_object>([&](blinded_balance_object &obj) {
                        obj.asset_id = o.amount.asset_id;
                        obj.owner = out.owner;
                        obj.commitment = out.commitment;
                    });
                }
                return void();
            } FC_CAPTURE_AND_RETHROW((o))
        }

        void transfer_from_blind_evaluator::do_apply(const transfer_from_blind_operation &o) {
            try {
                const auto &d = db();
                const auto &bbi = d.get_index<blinded_balance_index>();
                const auto &cidx = bbi.indices().get<by_commitment>();
                for (const auto &in : o.inputs) {
                    auto itr = cidx.find(in.commitment);
                    FC_ASSERT(itr != cidx.end());
                    FC_ASSERT(itr->asset_id == o.fee.asset_id);
                    FC_ASSERT(itr->owner == in.owner);
                }
                return void();
            } FC_CAPTURE_AND_RETHROW((o))

            try {
                db().adjust_balance(o.fee_payer(), o.fee);
                db().adjust_balance(o.to, o.amount);
                const auto &bbi = db().get_index<blinded_balance_index>();
                const auto &cidx = bbi.indices().get<by_commitment>();
                for (const auto &in : o.inputs) {
                    auto itr = cidx.find(in.commitment);
                    FC_ASSERT(itr != cidx.end());
                    db().remove(*itr);
                }
                const auto &add = o.amount.asset_id(db()).dynamic_asset_data_id(db());  // verify fee is a legit asset
                db().modify(add, [&](asset_dynamic_data_object &obj) {
                    obj.confidential_supply -= o.amount.amount + o.fee.amount;
                    FC_ASSERT(obj.confidential_supply >= 0);
                });
                return void();
            } FC_CAPTURE_AND_RETHROW((o))
        }

        void blind_transfer_evaluator::do_apply(const blind_transfer_operation &o) {
            try {
                const auto &d = db();
                o.fee.asset_id(db());  // verify fee is a legit asset
                const auto &bbi = db().get_index<blinded_balance_index>();
                const auto &cidx = bbi.indices().get<by_commitment>();
                for (const auto &out : o.outputs) {
                    for (const auto &a : out.owner.account_auths) {
                        a.first(d);
                    } // verify all accounts exist and are valid
                }
                for (const auto &in : o.inputs) {
                    auto itr = cidx.find(in.commitment);
                    STEEMIT_ASSERT(itr != cidx.end(), blind_transfer_unknown_commitment, "",
                                    ("commitment", in.commitment));
                    FC_ASSERT(itr->asset_id == o.fee.asset_id);
                    FC_ASSERT(itr->owner == in.owner);
                }
                return void();
            } FC_CAPTURE_AND_RETHROW((o))

            try {
                const auto &bbi = db().get_index<blinded_balance_index>();
                const auto &cidx = bbi.indices().get<by_commitment>();
                for (const auto &in : o.inputs) {
                    auto itr = cidx.find(in.commitment);
                    STEEMIT_ASSERT(itr != cidx.end(), blind_transfer_unknown_commitment, "",
                                    ("commitment", in.commitment));
                    db().remove(*itr);
                }
                for (const auto &out : o.outputs) {
                    db().create<blinded_balance_object>([&](blinded_balance_object &obj) {
                        obj.asset_id = o.fee.asset_id;
                        obj.owner = out.owner;
                        obj.commitment = out.commitment;
                    });
                }
                const auto &add = o.fee.asset_id(db()).dynamic_asset_data_id(db());
                db().modify(add, [&](asset_dynamic_data_object &obj) {
                    obj.confidential_supply -= o.fee.amount;
                    FC_ASSERT(obj.confidential_supply >= 0);
                });

                return void();
            } FC_CAPTURE_AND_RETHROW((o))
        }
    }
} // steemit::chain
