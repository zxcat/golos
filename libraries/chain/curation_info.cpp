#include <golos/chain/curation_info.hpp>

namespace golos { namespace chain {

    struct calculate_weight_helper {
        const database& db;
        const comment_object& comment;

        share_type abs_rshares;
        share_type vote_rshares;

        uint64_t old_vote_weight = 0;

        protocol::curation_curve curve = protocol::curation_curve::detect;

        /**
         *  weight / total_vote_weight ==> % of rshares increase that is accounted for by the vote
         *
         *  The equation for an individual vote is:
         *    W(R_N) - W(R_N-1), which is the delta increase of proportional weight
         *
         *  total_vote_weight =
         *    W(R_1) - W(R_0) +
         *    W(R_2) - W(R_1) + ...
         *    W(R_N) - W(R_N-1) = W(R_N) - W(R_0)
         *
         *  Since W(R_0) = 0, total_vote_weight is also bounded above by B and will always fit in a 64 bit integer.
         **/

        uint64_t calculate_weight(const comment_vote_object& vote) {
            if (vote.orig_rshares <= 0 || !comment.allow_curation_rewards) {
                return 0;
            }

            vote_rshares += vote.orig_rshares;

            if (vote.num_changes != 0 && vote.num_changes != -1) {
                return 0;
            }

            switch (curve) {
                case protocol::curation_curve::reverse_auction:
                    return calculate_reverse_auction(vote);

                case protocol::curation_curve::fractional:
                    return calculate_fractional(vote);

                case protocol::curation_curve::linear:
                    return calculate_linear(vote);

                case protocol::curation_curve::square_root:
                    return calculate_square_root(vote);

                default:
                    FC_ASSERT(false, "Unknown curation reward curve.");
            }
        }

        protocol::curation_curve detect_curation_curve() {
            curve = comment.curation_curve;

            if (protocol::curation_curve::detect == curve) {
                curve = protocol::curation_curve::fractional;
                //if (db.has_hardfork(STEEMIT_HARDFORK_0_19__677)) {
                    // TODO: select curve
                //}
            }

            return curve;
        }

    private:
        uint64_t calculate_reverse_auction(const comment_vote_object& vote) {
            abs_rshares += vote.orig_rshares; // there were no down-votes

            u512 rshares3(vote.orig_rshares);
            u256 total2(abs_rshares.value);

            rshares3 *= 10000;
            total2 *= 10000;

            rshares3 = rshares3 * rshares3 * rshares3;

            total2 *= total2;

            return static_cast<uint64_t>(rshares3 / total2);
        }

        /**
         * W(R) = B * R / ( R + 2S )
         *  W(R) is bounded above by B. B is fixed at 2^64 - 1, so all weights fit in a 64 bit integer.
         */
        uint64_t calculate_fractional(const comment_vote_object& vote) {
            static auto constant_alfa = uint128_t(2) * db.get_content_constant_s();

            uint64_t new_weight = (
                (uint128_t(vote_rshares.value) * std::numeric_limits<uint64_t>::max()) /
                (constant_alfa + vote_rshares.value)
            ).to_uint64();

            auto weight = new_weight - old_vote_weight;
            old_vote_weight = new_weight;

            return weight;
        }

        uint64_t calculate_linear(const comment_vote_object& vote) {
            return vote.rshares;
        }

        uint64_t calculate_square_root(const comment_vote_object& vote) {
             uint64_t new_weight(approx_sqrt(uint128_t(vote.rshares)));

             auto weight = new_weight - old_vote_weight;
             old_vote_weight = new_weight;

             return weight;
        }

        uint8_t find_msb(const uint128_t& u) const {
            uint64_t x;
            uint8_t places;
            x = (u.lo ? u.lo : 1);
            places = (u.hi ?   64 : 0);
            x = (u.hi ? u.hi : x);
            return uint8_t(boost::multiprecision::detail::find_msb(x) + places);
        }

        uint64_t approx_sqrt(const uint128_t& x) const {
            if ((x.lo == 0) && (x.hi == 0)) {
                return 0;
            }

            uint8_t msb_x = find_msb(x);
            uint8_t msb_z = msb_x >> 1;

            uint128_t msb_x_bit = uint128_t(1) << msb_x;
            uint64_t  msb_z_bit = uint64_t (1) << msb_z;

            uint128_t mantissa_mask = msb_x_bit - 1;
            uint128_t mantissa_x = x & mantissa_mask;
            uint64_t mantissa_z_hi = (msb_x & 1) ? msb_z_bit : 0;
            uint64_t mantissa_z_lo = (mantissa_x >> (msb_x - msb_z)).lo;
            uint64_t mantissa_z = (mantissa_z_hi | mantissa_z_lo) >> 1;
            uint64_t result = msb_z_bit | mantissa_z;

            return result;
        }
    }; // struct calculate_weight_helper

    comment_curation_info::comment_curation_info(database& db, const comment_object& comment, bool full_list)
    : comment(comment)
    {
        calculate_weight_helper helper{db, comment};
        curve = helper.detect_curation_curve();

        const auto& idx = db.get_index<comment_vote_index>().indices().get<by_comment_vote_order>();
        auto itr = idx.lower_bound(comment.id);
        auto etr = idx.end();

        vote_list.reserve(comment.total_votes);
        for (; etr != itr && itr->comment == comment.id; ++itr) {
            auto weight = helper.calculate_weight(*itr);

            if (comment.last_payout == fc::time_point_sec()) {
                total_vote_weight += weight;
            }

            if (weight > 0 && itr->auction_percent > 0) {
                auto new_weight = (uint128_t(weight) * itr->auction_percent / STEEMIT_100_PERCENT).to_uint64();

                if (comment.last_payout == fc::time_point_sec()) {
                    auction_window_weight += (weight - new_weight);
                    votes_in_auction_window_weight += new_weight;
                }

                weight = new_weight;
            }

            if (weight > 0 || full_list) {
                comment_vote_info vote{&(*itr), weight};
                vote_list.emplace_back(std::move(vote));
            }
        }

        votes_after_auction_window_weight = total_vote_weight - votes_in_auction_window_weight - auction_window_weight;

        std::sort(vote_list.begin(), vote_list.end(), [](auto& r, auto& l) {
            if (r.weight == l.weight) {
                return r.vote->voter < l.vote->voter;
            }
            return r.weight > l.weight;
        });
    }

} } // namespace golos::chain