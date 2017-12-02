#include <golos/chain/utilities/reward.hpp>
#include <golos/chain/utilities/uint256.hpp>

#define INV_LOG2_E_Q1DOT31  UINT64_C(0x58b90bfc) // Inverse log base 2 of e
#define INV_LOG2_10_Q1DOT31 UINT64_C(0x268826a1) // Inverse log base 2 of 10

namespace golos {
    namespace chain {
        namespace utilities {
            uint8_t find_msb(const fc::uint128_t &u) {
                uint64_t x;
                uint8_t places;
                x = (u.lo ? u.lo : 1);
                places = (u.hi ? 64 : 0);
                x = (u.hi ? u.hi : x);
                return uint8_t(boost::multiprecision::detail::find_msb(x) + places);
            }

            uint64_t approx_sqrt(const fc::uint128_t &x) {
                if ((x.lo == 0) && (x.hi == 0)) {
                    return 0;
                }

                uint8_t msb_x = find_msb(x);
                uint8_t msb_z = msb_x >> 1;

                fc::uint128_t msb_x_bit = fc::uint128_t(1) << msb_x;
                uint64_t msb_z_bit = uint64_t(1) << msb_z;

                fc::uint128_t mantissa_mask = msb_x_bit - 1;
                fc::uint128_t mantissa_x = x & mantissa_mask;
                uint64_t mantissa_z_hi = (msb_x & 1) ? msb_z_bit : 0;
                uint64_t mantissa_z_lo = (mantissa_x >> (msb_x - msb_z)).lo;
                uint64_t mantissa_z = (mantissa_z_hi | mantissa_z_lo) >> 1;
                uint64_t result = msb_z_bit | mantissa_z;

                return result;
            }

            //C. S. Turner,  "A Fast Binary Logarithm Algorithm", IEEE Signal Processing Mag., pp. 124,140, Sep. 2010.

            template<typename T>
            T approx_log2(const T &x, uint8_t type_size = sizeof(T) * 8, uint8_t exponenta_bits = 1) {
                if (x.lo <= 1 && x.hi == 0) {
                    return x.lo;
                }

                uint8_t mantissa_bits = type_size - exponenta_bits;
                T mantissa_mask = (1 << mantissa_bits) - 1;

                uint8_t msb = find_msb(x);
                uint8_t mantissa_shift = mantissa_bits - msb;

                return ((msb << mantissa_bits) | ((x << mantissa_shift) & mantissa_mask));
            }

            fc::uint128_t approx_log10(const fc::uint128_t &x) {
                return (approx_log2(x) * INV_LOG2_10_Q1DOT31) >> 31;
            }

            uint64_t get_rshare_reward(const comment_reward_context &ctx) {
                try {
                    FC_ASSERT(ctx.rshares > 0);
                    FC_ASSERT(ctx.total_reward_shares2 > 0);

                    boost::multiprecision::uint256_t rs(ctx.rshares.value);
                    boost::multiprecision::uint256_t rf(ctx.total_reward_fund_steem.amount.value);
                    boost::multiprecision::uint256_t total_rshares2 = to256(ctx.total_reward_shares2);

                    boost::multiprecision::uint256_t rs2 = to256(evaluate_reward_curve(ctx.rshares.value, ctx.reward_curve, ctx.content_constant));
                    rs2 = (rs2 * ctx.reward_weight) / STEEMIT_100_PERCENT;

                    boost::multiprecision::uint256_t payout_u256 = (rf * rs2) / total_rshares2;
                    FC_ASSERT(payout_u256 <=
                              boost::multiprecision::uint256_t(uint64_t(std::numeric_limits<int64_t>::max())));
                    uint64_t payout = static_cast< uint64_t >( payout_u256 );

                    if (is_comment_payout_dust(ctx.current_steem_price, payout)) {
                        payout = 0;
                    }

                    protocol::asset<0, 17, 0> max_steem = to_steem(ctx.current_steem_price, ctx.max_sbd);

                    payout = std::min(payout, uint64_t(max_steem.amount.value));

                    return payout;
                } FC_CAPTURE_AND_RETHROW((ctx))
            }

            uint64_t get_rshare_reward(const comment_reward_context &ctx, const reward_fund_object &rf_object) {
                try {
                    FC_ASSERT(ctx.rshares > 0);
                    FC_ASSERT(ctx.total_reward_shares2 > 0);

                    boost::multiprecision::uint256_t rs(ctx.rshares.value);
                    boost::multiprecision::uint256_t rf(ctx.total_reward_fund_steem.amount.value);
                    boost::multiprecision::uint256_t total_rshares2 = to256(ctx.total_reward_shares2);

                    //idump( (ctx) );

                    boost::multiprecision::uint256_t claim = to256(evaluate_reward_curve(ctx.rshares.value, ctx.reward_curve, ctx.content_constant));
                    rs2 = (rs2 * ctx.reward_weight) / STEEMIT_100_PERCENT;

                    boost::multiprecision::uint256_t payout_u256 = (rf * rs2) / total_rshares2;
                    FC_ASSERT(payout_u256 <=
                              boost::multiprecision::uint256_t(uint64_t(std::numeric_limits<int64_t>::max())));
                    uint64_t payout = static_cast< uint64_t >( payout_u256 );

                    if (is_comment_payout_dust(ctx.current_steem_price, payout)) {
                        payout = 0;
                    }

                    protocol::asset<0, 17, 0> max_steem = to_steem(ctx.current_steem_price, ctx.max_sbd);

                    payout = std::min(payout, uint64_t(max_steem.amount.value));

                    return payout;
                } FC_CAPTURE_AND_RETHROW((ctx))
            }

            uint64_t get_vote_weight(uint64_t vote_rshares, const reward_fund_object &rf) {
                uint64_t result = 0;
                if (rf.name == STEEMIT_POST_REWARD_FUND_NAME || rf.name == STEEMIT_COMMENT_REWARD_FUND_NAME) {
                    fc::uint128_t two_alpha = rf.content_constant * 2;
                    result = (fc::uint128_t(vote_rshares, 0) / (two_alpha + vote_rshares)).to_uint64();
                } else {
                    wlog("Unknown reward fund type ${rf}", ("rf", rf.name));
                }

                return result;
            }

            fc::uint128_t calculate_claims(const fc::uint128_t &rshares) {
                fc::uint128_t s = get_content_constant_s();
                fc::uint128_t rshares_plus_s = rshares + s;
                return rshares_plus_s * rshares_plus_s - s * s;
            }

            fc::uint128_t calculate_claims(const fc::uint128_t &rshares, const reward_fund_object &rf) {
                fc::uint128_t result = 0;
                if (rf.name == STEEMIT_POST_REWARD_FUND_NAME || rf.name == STEEMIT_COMMENT_REWARD_FUND_NAME) {
                    fc::uint128_t s = rf.content_constant;
                    fc::uint128_t rshares_plus_s = rshares + s;
                    result = rshares_plus_s * rshares_plus_s - s * s;
                } else {
                    wlog("Unknown reward fund type ${rf}", ("rf", rf.name));
                }

                return result;
            }

            fc::uint128_t evaluate_reward_curve(const fc::uint128_t &rshares, const protocol::reward_curve &curve,
                                                const fc::uint128_t &content_constant) {
                fc::uint128_t result = 0;

                if (curve == protocol::reward_curve::quadratic) {
                    fc::uint128_t rshares_plus_s = rshares + content_constant;
                    result = rshares_plus_s * rshares_plus_s - content_constant * content_constant;
                } else if (curve == protocol::reward_curve::quadratic_curation) {
                    fc::uint128_t two_alpha = content_constant * 2;
                    result = uint128_t(rshares.lo, 0) / (two_alpha + rshares);
                } else if (curve == protocol::reward_curve::log10) {
                    result = approx_log10(rshares);
                } else if (curve == protocol::reward_curve::log2) {
                    result = approx_log2(rshares);
                } else if (curve == protocol::reward_curve::linear) {
                    result = rshares;
                } else if (curve == protocol::reward_curve::square_root) {
                    result = approx_sqrt(rshares);
                }

                return result;
            }
        }
    }
}