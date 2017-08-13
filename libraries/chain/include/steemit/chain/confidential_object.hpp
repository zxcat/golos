#pragma once

#include <steemit/chain/protocol/authority.hpp>
#include <steemit/chain/protocol/types.hpp>

#include <fc/crypto/elliptic.hpp>

namespace steemit {
    namespace chain {

        /**
         * @class blinded_balance_object
         * @brief tracks a blinded balance commitment
         * @ingroup object
         * @ingroup protocol
         */
        class blinded_balance_object : public object<blinded_balance_object_type, blinded_balance_object> {
        public:
            fc::ecc::commitment_type commitment;
            asset_name_type asset_id;
            authority owner;
        };

        struct by_asset;
        struct by_owner;
        struct by_commitment;

        /**
         * @ingroup object_index
         */
        typedef multi_index_container <blinded_balance_object, indexed_by<ordered_unique < tag < by_id>, member<object,
                object_id_type, &object::id>>,
        ordered_unique <tag<by_commitment>, member<blinded_balance_object, commitment_type,
                &blinded_balance_object::commitment>>
        >
        >
        blinded_balance_object_multi_index_type;
        typedef generic_index <blinded_balance_object, blinded_balance_object_multi_index_type> blinded_balance_index;

    }
} // steemit::chain

FC_REFLECT_DERIVED(steemit::chain::blinded_balance_object, (commitment)(asset_id)(owner))
