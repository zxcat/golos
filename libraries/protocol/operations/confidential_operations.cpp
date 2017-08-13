#include <steemit/protocol/operations/confidential_operations.hpp>

#include <steemit/chain/confidential_evaluator.hpp>
#include <steemit/chain/database.hpp>

#include <fc/crypto/base58.hpp>
#include <fc/io/raw.hpp>
#include <fc/crypto/elliptic.hpp>

namespace steemit {
    namespace protocol {

        void transfer_to_blind_operation::validate() const {
            FC_ASSERT(amount.amount > 0);

            vector<commitment_type> in;
            vector<commitment_type> out(outputs.size());
            int64_t net_public = amount.amount.value;
            for (uint32_t i = 0; i < out.size(); ++i) {
                out[i] = outputs[i].commitment;
                /// require all outputs to be sorted prevents duplicates AND prevents implementations
                /// from accidentally leaking information by how they arrange commitments.
                if (i > 0)
                    FC_ASSERT(out[i - 1] < out[i], "all outputs must be sorted by commitment id");
                FC_ASSERT(!outputs[i].owner.is_impossible());
            }
            FC_ASSERT(out.size(), "there must be at least one output");

            auto public_c = fc::ecc::blind(blinding_factor, net_public);

            FC_ASSERT(fc::ecc::verify_sum({public_c}, out, 0), "", ("net_public", net_public));

            if (outputs.size() > 1) {
                for (auto out : outputs) {
                    auto info = fc::ecc::range_get_info(out.range_proof);
                    FC_ASSERT(info.max_value <= STEEMIT_MAX_SHARE_SUPPLY);
                }
            }
        }

        void transfer_from_blind_operation::validate() const {
            FC_ASSERT(amount.amount > 0);
            FC_ASSERT(inputs.size() > 0);


            vector<commitment_type> in(inputs.size());
            vector<commitment_type> out;
            out.push_back(fc::ecc::blind(blinding_factor, 0));
            for (uint32_t i = 0; i < in.size(); ++i) {
                in[i] = inputs[i].commitment;
                /// by requiring all inputs to be sorted we also prevent duplicate commitments on the input
                if (i > 0)
                    FC_ASSERT(in[i - 1] < in[i], "all inputs must be sorted by commitment id");
            }
            FC_ASSERT(in.size(), "there must be at least one input");
            FC_ASSERT(fc::ecc::verify_sum(in, out, 0));
        }

        /**
         *  This method can be computationally intensive because it verifies that input commitments - output commitments add up to 0
         */
        void blind_transfer_operation::validate() const {
            try {
                vector<commitment_type> in(inputs.size());
                vector<commitment_type> out(outputs.size());
                for (uint32_t i = 0; i < in.size(); ++i) {
                    in[i] = inputs[i].commitment;
                    /// by requiring all inputs to be sorted we also prevent duplicate commitments on the input
                    if (i > 0)
                        FC_ASSERT(in[i - 1] < in[i]);
                }
                for (uint32_t i = 0; i < out.size(); ++i) {
                    out[i] = outputs[i].commitment;
                    if (i > 0)
                        FC_ASSERT(out[i - 1] < out[i]);
                    FC_ASSERT(!outputs[i].owner.is_impossible());
                }
                FC_ASSERT(in.size(), "there must be at least one input");
                FC_ASSERT(fc::ecc::verify_sum(in, out, net_public), "", ("net_public", net_public));

                if (outputs.size() > 1) {
                    for (auto out : outputs) {
                        auto info = fc::ecc::range_get_info(out.range_proof);
                        FC_ASSERT(info.max_value <= STEEMIT_MAX_SHARE_SUPPLY);
                    }
                }
            } FC_CAPTURE_AND_RETHROW((*this))
        }

        /**
         *  Packs *this then encodes as base58 encoded string.
         */
        stealth_confirmation::operator string() const {
            return fc::to_base58(fc::raw::pack(*this));
        }

        /**
         * Unpacks from a base58 string
         */
        stealth_confirmation::stealth_confirmation(const std::string &base58) {
            *this = fc::raw::unpack<stealth_confirmation>(fc::from_base58(base58));
        }
    }
} // graphene::chain
