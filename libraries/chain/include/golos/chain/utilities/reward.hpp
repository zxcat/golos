#pragma once

#include <golos/chain/utilities/asset.hpp>
#include <golos/chain/objects/steem_objects.hpp>

#include <golos/protocol/asset.hpp>
#include <golos/protocol/config.hpp>
#include <golos/protocol/types.hpp>

#include <fc/reflect/reflect.hpp>

#include <fc/uint128_t.hpp>

namespace golos {
    namespace chain {
        namespace utilities {
            struct comment_reward_context {
                protocol::share_type rshares;
                uint16_t reward_weight = 0;
                protocol::asset<0, 17, 0> max_sbd;
                fc::uint128_t total_reward_shares2;
                protocol::asset<0, 17, 0> total_reward_fund_steem;
                protocol::price<0, 17, 0> current_steem_price;
                protocol::reward_curve reward_curve = protocol::reward_curve::quadratic;
                uint128_t content_constant = STEEMIT_CONTENT_CONSTANT_HF0;
            };

            uint64_t get_rshare_reward(const comment_reward_context &ctx);

            uint64_t get_rshare_reward(const comment_reward_context &ctx, const reward_fund_object &rf);

            uint64_t get_vote_weight(uint64_t vote_rshares, const reward_fund_object &rf);

            inline fc::uint128_t get_content_constant_s() {
                return STEEMIT_CONTENT_CONSTANT_HF0; // looking good for posters
            }

            fc::uint128_t calculate_claims(const fc::uint128_t &rshares);

            fc::uint128_t calculate_claims(const fc::uint128_t &rshares, const reward_fund_object &rf);

            uint128_t evaluate_reward_curve(const uint128_t &rshares,
                                            const protocol::reward_curve &curve = protocol::reward_curve::quadratic,
                                            const uint128_t &content_constant = STEEMIT_CONTENT_CONSTANT_HF0);

            inline bool is_comment_payout_dust(const protocol::price<0, 17, 0> &p, uint64_t steem_payout) {
                return to_sbd(p, protocol::asset<0, 17, 0>(steem_payout, STEEM_SYMBOL_NAME)) < STEEMIT_MIN_PAYOUT_SBD;
            }
        }
    }
}

FC_REFLECT((golos::chain::utilities::comment_reward_context),
           (rshares)(reward_weight)(max_sbd)(total_reward_shares2)(total_reward_fund_steem)(current_steem_price))

