#pragma once

#include <steemit/app/plugin.hpp>

#include <steemit/chain/steem_object_types.hpp>

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
#ifndef BLOCKCHAIN_STATISTICS_SPACE_ID
#define BLOCKCHAIN_STATISTICS_SPACE_ID 9
#endif

#ifndef BLOCKCHAIN_STATISTICS_PLUGIN_NAME
#define BLOCKCHAIN_STATISTICS_PLUGIN_NAME "chain_stats"
#endif

namespace steemit {
    namespace blockchain_statistics {

        using namespace steemit::chain;
        using app::application;

        enum blockchain_statistics_object_type {
            bucket_object_type = (BLOCKCHAIN_STATISTICS_SPACE_ID << 8)
        };

        namespace detail {
            class blockchain_statistics_plugin_impl;
            // std::vector < std::string > calculate_buckets_delta (const bucket_object & a, const bucket_object & b);
        }

        class blockchain_statistics_plugin : public steemit::app::plugin {
        public:
            blockchain_statistics_plugin(application *app);

            virtual ~blockchain_statistics_plugin();

            virtual std::string plugin_name() const override {
                return BLOCKCHAIN_STATISTICS_PLUGIN_NAME;
            }

            virtual void plugin_set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg) override;

            virtual void plugin_initialize(const boost::program_options::variables_map &options) override;

            virtual void plugin_startup() override;

            const flat_set<uint32_t> &get_tracked_buckets() const;

            uint32_t get_max_history_per_bucket() const;

        private:
            friend class detail::blockchain_statistics_plugin_impl;

            std::unique_ptr<detail::blockchain_statistics_plugin_impl> _my;
        };

        struct bucket_object
                : public object<bucket_object_type, bucket_object> {
            template<typename Constructor, typename Allocator>
            bucket_object(Constructor &&c, allocator<Allocator> a) {
                c(*this);
            }

            id_type id;

            fc::time_point_sec open;                                        ///< Open time of the bucket
            uint32_t seconds = 0;                                 ///< Seconds accounted for in the bucket
            uint32_t blocks = 0;                                  ///< Blocks produced
            uint32_t bandwidth = 0;                               ///< Bandwidth in bytes
            uint32_t operations = 0;                              ///< Operations evaluated
            uint32_t transactions = 0;                            ///< Transactions processed
            uint32_t transfers = 0;                               ///< Account to account transfers
            share_type steem_transferred = 0;                       ///< STEEM transferred from account to account
            share_type sbd_transferred = 0;                         ///< SBD transferred from account to account
            share_type sbd_paid_as_interest = 0;                    ///< SBD paid as interest
            uint32_t paid_accounts_created = 0;                   ///< Accounts created with fee
            uint32_t mined_accounts_created = 0;                  ///< Accounts mined for free
            uint32_t root_comments = 0;                           ///< Top level root comments
            uint32_t root_comment_edits = 0;                      ///< Edits to root comments
            uint32_t root_comments_deleted = 0;                   ///< Root comments deleted
            uint32_t replies = 0;                                 ///< Replies to comments
            uint32_t reply_edits = 0;                             ///< Edits to replies
            uint32_t replies_deleted = 0;                         ///< Replies deleted
            uint32_t new_root_votes = 0;                          ///< New votes on root comments
            uint32_t changed_root_votes = 0;                      ///< Changed votes on root comments
            uint32_t new_reply_votes = 0;                         ///< New votes on replies
            uint32_t changed_reply_votes = 0;                     ///< Changed votes on replies
            uint32_t payouts = 0;                                 ///< Number of comment payouts
            share_type sbd_paid_to_authors = 0;                     ///< Ammount of SBD paid to authors
            share_type vests_paid_to_authors = 0;                   ///< Ammount of VESS paid to authors
            share_type vests_paid_to_curators = 0;                  ///< Ammount of VESTS paid to curators
            share_type liquidity_rewards_paid = 0;                  ///< Ammount of STEEM paid to market makers
            uint32_t transfers_to_vesting = 0;                    ///< Transfers of STEEM into VESTS
            share_type steem_vested = 0;                            ///< Ammount of STEEM vested
            uint32_t new_vesting_withdrawal_requests = 0;         ///< New vesting withdrawal requests
            uint32_t modified_vesting_withdrawal_requests = 0;    ///< Changes to vesting withdrawal requests
            share_type vesting_withdraw_rate_delta = 0;
            uint32_t vesting_withdrawals_processed = 0;           ///< Number of vesting withdrawals
            uint32_t finished_vesting_withdrawals = 0;            ///< Processed vesting withdrawals that are now finished
            share_type vests_withdrawn = 0;                         ///< Ammount of VESTS withdrawn to STEEM
            share_type vests_transferred = 0;                       ///< Ammount of VESTS transferred to another account
            uint32_t sbd_conversion_requests_created = 0;         ///< SBD conversion requests created
            share_type sbd_to_be_converted = 0;                     ///< Amount of SBD to be converted
            uint32_t sbd_conversion_requests_filled = 0;          ///< SBD conversion requests filled
            share_type steem_converted = 0;                         ///< Amount of STEEM that was converted
            uint32_t limit_orders_created = 0;                    ///< Limit orders created
            uint32_t limit_orders_filled = 0;                     ///< Limit orders filled
            uint32_t limit_orders_cancelled = 0;                  ///< Limit orders cancelled
            uint32_t total_pow = 0;                               ///< POW submitted
            uint128_t estimated_hashpower = 0;                     ///< Estimated average hashpower over interval
        };

        // std::vector < std::string > calculate_buckets_delta (const bucket_object & a, const bucket_object & b) {
        //     std::vector < std::string > result;
        //     if (a.seconds - b.seconds != 0 ) {
        //         result.push_back("seconds:" + std::to_string(a.seconds - b.seconds) + "|c");
        //     }
        //     if (a.blocks - b.blocks != 0 ) {
        //         result.push_back("blocks:" + std::to_string(a.blocks - b.blocks) + "|c");
        //     }
        //     if (a.bandwidth - b.bandwidth != 0 ) {
        //         result.push_back("bandwidth:" + std::to_string(a.bandwidth - b.bandwidth) + "|c");
        //     }
        //     if (a.operations - b.operations != 0 ) {
        //         result.push_back("operations:" + std::to_string(a.operations - b.operations) + "|c");
        //     }
        //     if (a.transactions - b.transactions != 0 ) {
        //         result.push_back("transactions:" + std::to_string(a.transactions - b.transactions) + "|c");
        //     }
        //     if (a.transfers - b.transfers != 0 ) {
        //         result.push_back("transfers:" + std::to_string(a.transfers - b.transfers) + "|c");
        //     }
        //     if (a.steem_transferred - b.steem_transferred != 0 ) {
        //         result.push_back("steem_transferred:" + std::string(a.steem_transferred - b.steem_transferred) + "|c");
        //     }
        //     if (a.sbd_transferred - b.sbd_transferred != 0 ) {
        //         result.push_back("sbd_transferred:" + std::string(a.sbd_transferred - b.sbd_transferred) + "|c");
        //     }
        //     if (a.sbd_paid_as_interest - b.sbd_paid_as_interest != 0 ) {
        //         result.push_back("sbd_paid_as_interest:" + std::string(a.sbd_paid_as_interest - b.sbd_paid_as_interest) + "|c");
        //     }
        //     if (a.paid_accounts_created - b.paid_accounts_created != 0 ) {
        //         result.push_back("paid_accounts_created:" + std::to_string(a.paid_accounts_created - b.paid_accounts_created) + "|c");
        //     }
        //     if (a.mined_accounts_created - b.mined_accounts_created != 0 ) {
        //         result.push_back("mined_accounts_created:" + std::to_string(a.mined_accounts_created - b.mined_accounts_created) + "|c");
        //     }
        //     if (a.root_comments - b.root_comments != 0 ) {
        //         result.push_back("root_comments:" + std::to_string(a.root_comments - b.root_comments) + "|c");
        //     }
        //     if (a.root_comment_edits - b.root_comment_edits != 0 ) {
        //         result.push_back("root_comment_edits:" + std::to_string(a.root_comment_edits - b.root_comment_edits) + "|c");
        //     }
        //     if (a.root_comments_deleted - b.root_comments_deleted != 0 ) {
        //         result.push_back("root_comments_deleted:" + std::to_string(a.root_comments_deleted - b.root_comments_deleted) + "|c");
        //     }
        //     if (a.replies - b.replies != 0 ) {
        //         result.push_back("replies:" + std::to_string(a.replies - b.replies) + "|c");
        //     }
        //     if (a.reply_edits - b.reply_edits != 0 ) {
        //         result.push_back("reply_edits:" + std::to_string(a.reply_edits - b.reply_edits) + "|c");
        //     }
        //     if (a.replies_deleted - b.replies_deleted != 0 ) {
        //         result.push_back("replies_deleted:" + std::to_string(a.replies_deleted - b.replies_deleted) + "|c");
        //     }
        //     if (a.new_root_votes - b.new_root_votes != 0 ) {
        //         result.push_back("new_root_votes:" + std::to_string(a.new_root_votes - b.new_root_votes) + "|c");
        //     }
        //     if (a.changed_root_votes - b.changed_root_votes != 0 ) {
        //         result.push_back("changed_root_votes:" + std::to_string(a.changed_root_votes - b.changed_root_votes) + "|c");
        //     }
        //     if (a.new_reply_votes - b.new_reply_votes != 0 ) {
        //         result.push_back("new_reply_votes:" + std::to_string(a.new_reply_votes - b.new_reply_votes) + "|c");
        //     }
        //     if (a.changed_reply_votes - b.changed_reply_votes != 0 ) {
        //         result.push_back("changed_reply_votes:" + std::to_string(a.changed_reply_votes - b.changed_reply_votes) + "|c");
        //     }
        //     if (a.payouts - b.payouts != 0 ) {
        //         result.push_back("payouts:" + std::to_string(a.payouts - b.payouts) + "|c");
        //     }
        //     if (a.sbd_paid_to_authors - b.sbd_paid_to_authors != 0 ) {
        //         result.push_back("sbd_paid_to_authors:" + std::string(a.sbd_paid_to_authors - b.sbd_paid_to_authors) + "|c");
        //     }
        //     if (a.vests_paid_to_authors - b.vests_paid_to_authors != 0 ) {
        //         result.push_back("vests_paid_to_authors:" + std::string(a.vests_paid_to_authors - b.vests_paid_to_authors) + "|c");
        //     }
        //     if (a.vests_paid_to_curators - b.vests_paid_to_curators != 0 ) {
        //         result.push_back("vests_paid_to_curators:" + std::string(a.vests_paid_to_curators - b.vests_paid_to_curators) + "|c");
        //     }
        //     if (a.liquidity_rewards_paid - b.liquidity_rewards_paid != 0 ) {
        //         result.push_back("liquidity_rewards_paid:" + std::string(a.liquidity_rewards_paid - b.liquidity_rewards_paid) + "|c");
        //     }
        //     if (a.transfers_to_vesting - b.transfers_to_vesting != 0 ) {
        //         result.push_back("transfers_to_vesting:" + std::to_string(a.transfers_to_vesting - b.transfers_to_vesting) + "|c");
        //     }
        //     if (a.steem_vested - b.steem_vested != 0 ) {
        //         result.push_back("steem_vested:" + std::string(a.steem_vested - b.steem_vested) + "|c");
        //     }
        //     if (a.new_vesting_withdrawal_requests - b.new_vesting_withdrawal_requests != 0 ) {
        //         result.push_back("new_vesting_withdrawal_requests:" + std::to_string(a.new_vesting_withdrawal_requests - b.new_vesting_withdrawal_requests) + "|c");
        //     }
        //     if (a.modified_vesting_withdrawal_requests - b.modified_vesting_withdrawal_requests != 0 ) {
        //         result.push_back("modified_vesting_withdrawal_requests:" + std::to_string(a.modified_vesting_withdrawal_requests - b.modified_vesting_withdrawal_requests) + "|c");
        //     }
        //     if (a.vesting_withdraw_rate_delta - b.vesting_withdraw_rate_delta != 0 ) {
        //         result.push_back("vesting_withdraw_rate_delta:" + std::string(a.vesting_withdraw_rate_delta - b.vesting_withdraw_rate_delta) + "|c");
        //     }
        //     if (a.vesting_withdrawals_processed - b.vesting_withdrawals_processed != 0 ) {
        //         result.push_back("vesting_withdrawals_processed:" + std::to_string(a.vesting_withdrawals_processed - b.vesting_withdrawals_processed) + "|c");
        //     }
        //     if (a.finished_vesting_withdrawals - b.finished_vesting_withdrawals != 0 ) {
        //         result.push_back("finished_vesting_withdrawals:" + std::to_string(a.finished_vesting_withdrawals - b.finished_vesting_withdrawals) + "|c");
        //     }
        //     if (a.vests_withdrawn - b.vests_withdrawn != 0 ) {
        //         result.push_back("vests_withdrawn:" + std::string(a.vests_withdrawn - b.vests_withdrawn) + "|c");
        //     }
        //     if (a.vests_transferred - b.vests_transferred != 0 ) {
        //         result.push_back("vests_transferred:" + std::string(a.vests_transferred - b.vests_transferred) + "|c");
        //     }
        //     if (a.sbd_conversion_requests_created - b.sbd_conversion_requests_created != 0 ) {
        //         result.push_back("sbd_conversion_requests_created:" + std::to_string(a.sbd_conversion_requests_created - b.sbd_conversion_requests_created) + "|c");
        //     }
        //     if (a.sbd_to_be_converted - b.sbd_to_be_converted != 0 ) {
        //         result.push_back("sbd_to_be_converted:" + std::string(a.sbd_to_be_converted - b.sbd_to_be_converted) + "|c");
        //     }
        //     if (a.sbd_conversion_requests_filled - b.sbd_conversion_requests_filled != 0 ) {
        //         result.push_back("sbd_conversion_requests_filled:" + std::to_string(a.sbd_conversion_requests_filled - b.sbd_conversion_requests_filled) + "|c");
        //     }
        //     if (a.steem_converted - b.steem_converted != 0 ) {
        //         result.push_back("steem_converted:" + std::string(a.steem_converted - b.steem_converted) + "|c");
        //     }
        //     if (a.limit_orders_created - b.limit_orders_created != 0 ) {
        //         result.push_back("limit_orders_created:" + std::to_string(a.limit_orders_created - b.limit_orders_created) + "|c");
        //     }
        //     if (a.limit_orders_filled - b.limit_orders_filled != 0 ) {
        //         result.push_back("limit_orders_filled:" + std::to_string(a.limit_orders_filled - b.limit_orders_filled) + "|c");
        //     }
        //     if (a.limit_orders_cancelled - b.limit_orders_cancelled != 0 ) {
        //         result.push_back("limit_orders_cancelled:" + std::to_string(a.limit_orders_cancelled - b.limit_orders_cancelled) + "|c");
        //     }
        //     if (a.total_pow - b.total_pow != 0 ) {
        //         result.push_back("total_pow:" + std::to_string(a.total_pow - b.total_pow) + "|c");
        //     }
        //     if (a.estimated_hashpower - b.estimated_hashpower != 0 ) {
        //         result.push_back("estimated_hashpower:" + std::string(a.estimated_hashpower - b.estimated_hashpower) + "|c");
        //     }
        //     return result;
        // }

        typedef object_id<bucket_object> bucket_id_type;

        struct by_id;
        struct by_bucket;
        typedef multi_index_container<
                bucket_object,
                indexed_by<
                        ordered_unique<tag<by_id>, member<bucket_object, bucket_id_type, &bucket_object::id>>,
                        ordered_unique<tag<by_bucket>,
                                composite_key<bucket_object,
                                        member<bucket_object, uint32_t, &bucket_object::seconds>,
                                        member<bucket_object, fc::time_point_sec, &bucket_object::open>
                                >
                        >
                >,
                allocator<bucket_object>
        > bucket_index;

    }
} // steemit::blockchain_statistics

FC_REFLECT(steemit::blockchain_statistics::bucket_object,
        (id)
                (open)
                (seconds)
                (blocks)
                (bandwidth)
                (operations)
                (transactions)
                (transfers)
                (steem_transferred)
                (sbd_transferred)
                (sbd_paid_as_interest)
                (paid_accounts_created)
                (mined_accounts_created)
                (root_comments)
                (root_comment_edits)
                (root_comments_deleted)
                (replies)
                (reply_edits)
                (replies_deleted)
                (new_root_votes)
                (changed_root_votes)
                (new_reply_votes)
                (changed_reply_votes)
                (payouts)
                (sbd_paid_to_authors)
                (vests_paid_to_authors)
                (vests_paid_to_curators)
                (liquidity_rewards_paid)
                (transfers_to_vesting)
                (steem_vested)
                (new_vesting_withdrawal_requests)
                (modified_vesting_withdrawal_requests)
                (vesting_withdraw_rate_delta)
                (vesting_withdrawals_processed)
                (finished_vesting_withdrawals)
                (vests_withdrawn)
                (vests_transferred)
                (sbd_conversion_requests_created)
                (sbd_to_be_converted)
                (sbd_conversion_requests_filled)
                (steem_converted)
                (limit_orders_created)
                (limit_orders_filled)
                (limit_orders_cancelled)
                (total_pow)
                (estimated_hashpower)
)
CHAINBASE_SET_INDEX_TYPE(steemit::blockchain_statistics::bucket_object, steemit::blockchain_statistics::bucket_index)
