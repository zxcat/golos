#include <steemit/blockchain_statistics/blockchain_statistics_api.hpp>

#include <steemit/app/impacted.hpp>
#include <steemit/chain/account_object.hpp>
#include <steemit/chain/comment_object.hpp>
#include <steemit/chain/history_object.hpp>

#include <steemit/chain/index.hpp>
#include <steemit/chain/operation_notification.hpp>

#include <memory>
#include <steemit/blockchain_statistics/statistics_sender.hpp>

namespace steemit {
namespace blockchain_statistics {
namespace detail {
    std::vector < std::string > calculate_buckets_delta (const bucket_object & a, const bucket_object & b);

    using namespace steemit::protocol;

    class blockchain_statistics_plugin_impl {
    public:
        blockchain_statistics_plugin_impl(blockchain_statistics_plugin &plugin)
                : _self(plugin) {
        }

        virtual ~blockchain_statistics_plugin_impl() {
        }

        void on_block(const signed_block &b);

        void pre_operation(const operation_notification &o);

        void post_operation(const operation_notification &o);

        blockchain_statistics_plugin &_self;
        flat_set<uint32_t> _tracked_buckets = {60, 3600, 21600, 86400,
                                               604800, 2592000
        };
        flat_set<bucket_id_type> _current_buckets;
        uint32_t _maximum_history_per_bucket_size = 100;
        std::shared_ptr<statistics_sender> stat_sender_;
    };

    struct operation_process {
        const blockchain_statistics_plugin &_plugin;
        const bucket_object &_bucket;
        std::shared_ptr<statistics_sender> stat_sender_;
        chain::database &_db;

        operation_process(
            blockchain_statistics_plugin &bsp,
            const bucket_object &b,
            std::shared_ptr<statistics_sender> stat_sender) :
            _plugin(bsp), _bucket(b), _db( bsp.database() ), stat_sender_(stat_sender) {
        }

        typedef void result_type;

        template<typename T>
        void operator()(const T &) const {
        }

        void operator()(const transfer_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.transfers++;

                if (op.amount.symbol == STEEM_SYMBOL) {
                    b.steem_transferred += op.amount.amount;
                } else {
                    b.sbd_transferred += op.amount.amount;
                }
            });
        }

        void operator()(const interest_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.sbd_paid_as_interest += op.interest.amount;
            });
        }

        void operator()(const account_create_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.paid_accounts_created++;
            });
        }

        void operator()(const pow_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                auto &worker = _db.get_account(op.worker_account);

                if (worker.created == _db.head_block_time()) {
                    b.mined_accounts_created++;
                }

                b.total_pow++;

                uint64_t bits =
                        (_db.get_dynamic_global_properties().num_pow_witnesses /
                         4) + 4;
                uint128_t estimated_hashes = (1 << bits);
                uint32_t delta_t;

                if (b.seconds == 0) {
                    delta_t = _db.head_block_time().sec_since_epoch() -
                              b.open.sec_since_epoch();
                } else {
                    delta_t = b.seconds;
                }

                b.estimated_hashpower =
                        (b.estimated_hashpower * delta_t +
                         estimated_hashes) / delta_t;
            });
        }

        void operator()(const comment_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                auto &comment = _db.get_comment(op.author, op.permlink);

                if (comment.created == _db.head_block_time()) {
                    if (comment.parent_author.length()) {
                        b.replies++;
                    } else {
                        b.root_comments++;
                    }
                } else {
                    if (comment.parent_author.length()) {
                        b.reply_edits++;
                    } else {
                        b.root_comment_edits++;
                    }
                }
            });
        }

        void operator()(const vote_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                const auto &cv_idx = _db.get_index<comment_vote_index>().indices().get<by_comment_voter>();
                auto &comment = _db.get_comment(op.author, op.permlink);
                auto &voter = _db.get_account(op.voter);
                auto itr = cv_idx.find(boost::make_tuple(comment.id, voter.id));

                if (itr->num_changes) {
                    if (comment.parent_author.size()) {
                        b.new_reply_votes++;
                    } else {
                        b.new_root_votes++;
                    }
                } else {
                    if (comment.parent_author.size()) {
                        b.changed_reply_votes++;
                    } else {
                        b.changed_root_votes++;
                    }
                }
            });
        }

        void operator()(const author_reward_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.payouts++;
                b.sbd_paid_to_authors += op.sbd_payout.amount;
                b.vests_paid_to_authors += op.vesting_payout.amount;
            });
        }

        void operator()(const curation_reward_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.vests_paid_to_curators += op.reward.amount;
            });
        }

        void operator()(const liquidity_reward_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.liquidity_rewards_paid += op.payout.amount;
            });
        }

        void operator()(const transfer_to_vesting_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.transfers_to_vesting++;
                b.steem_vested += op.amount.amount;
            });
        }

        void operator()(const fill_vesting_withdraw_operation &op) const {
            auto &account = _db.get_account(op.from_account);

            _db.modify(_bucket, [&](bucket_object &b) {
                b.vesting_withdrawals_processed++;
                if (op.deposited.symbol == STEEM_SYMBOL) {
                    b.vests_withdrawn += op.withdrawn.amount;
                } else {
                    b.vests_transferred += op.withdrawn.amount;
                }

                if (account.vesting_withdraw_rate.amount == 0) {
                    b.finished_vesting_withdrawals++;
                }
            });
        }

        void operator()(const limit_order_create_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.limit_orders_created++;
            });
        }

        void operator()(const fill_order_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.limit_orders_filled += 2;
            });
        }

        void operator()(const limit_order_cancel_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.limit_orders_cancelled++;
            });
        }

        void operator()(const convert_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.sbd_conversion_requests_created++;
                b.sbd_to_be_converted += op.amount.amount;
            });
        }

        void operator()(const fill_convert_request_operation &op) const {
            _db.modify(_bucket, [&](bucket_object &b) {
                b.sbd_conversion_requests_filled++;
                b.steem_converted += op.amount_out.amount;
            });
        }
    };

    void blockchain_statistics_plugin_impl::on_block(const signed_block &b) {
        auto &db = _self.database();

        if (b.block_num() == 1) {
            db.create<bucket_object>([&](bucket_object &bo) {
                bo.open = b.timestamp;
                bo.seconds = 0;
                bo.blocks = 1;
            });
        } else {
            db.modify(db.get(bucket_id_type()), [&](bucket_object &bo) {
                bo.blocks++;
            });
        }
        auto old_global_bucket = db.get( bucket_id_type() );
        // GET old bucket here

        _current_buckets.clear();
        _current_buckets.insert(bucket_id_type());

        const auto &bucket_idx = db.get_index<bucket_index>().indices().get<by_bucket>();

        uint32_t trx_size = 0;
        uint32_t num_trx = b.transactions.size();

        for (auto trx : b.transactions) {
            trx_size += fc::raw::pack_size(trx);
        }


        for (auto bucket : _tracked_buckets) {
            auto open = fc::time_point_sec(
                    (db.head_block_time().sec_since_epoch() / bucket) *
                    bucket);
            auto itr = bucket_idx.find(boost::make_tuple(bucket, open));
            // чекается  время. надо обновлять али нет

            if (itr == bucket_idx.end()) {
                _current_buckets.insert(
                        db.create<bucket_object>([&](bucket_object &bo) {
                            bo.open = open;
                            bo.seconds = bucket;
                            bo.blocks = 1;
                        }).id);

                if (_maximum_history_per_bucket_size > 0) {
                    try {
                        auto cutoff = fc::time_point_sec((
                                safe<uint32_t>(db.head_block_time().sec_since_epoch()) -
                                safe<uint32_t>(bucket) *
                                safe<uint32_t>(_maximum_history_per_bucket_size)).value);

                        itr = bucket_idx.lower_bound(boost::make_tuple(bucket, fc::time_point_sec()));

                        while (itr->seconds == bucket &&
                               itr->open < cutoff) {
                            auto old_itr = itr;
                            ++itr;
                            db.remove(*old_itr);
                        }
                    }
                    catch (fc::overflow_exception &e) {
                    }
                    catch (fc::underflow_exception &e) {
                    }
                }
            } else {
                db.modify(*itr, [&](bucket_object &bo) {
                    bo.blocks++;
                });

                _current_buckets.insert(itr->id);
            }

            db.modify(*itr, [&](bucket_object &bo) {
                bo.transactions += num_trx;
                bo.bandwidth += trx_size;
            });
        }

        auto new_global_bucket = db.get( bucket_id_type() );
        auto delta_strings = calculate_buckets_delta(old_global_bucket, new_global_bucket);

        for ( auto tmp_str : delta_strings ) {
            stat_sender_->push (tmp_str);
        }
    }

    void blockchain_statistics_plugin_impl::pre_operation(const operation_notification &o) {
        auto &db = _self.database();

        for (auto bucket_id : _current_buckets) {
            if (o.op.which() ==
                operation::tag<delete_comment_operation>::value) {
                delete_comment_operation op = o.op.get<delete_comment_operation>();
                auto comment = db.get_comment(op.author, op.permlink);
                const auto &bucket = db.get(bucket_id);

                db.modify(bucket, [&](bucket_object &b) {
                    if (comment.parent_author.length()) {
                        b.replies_deleted++;
                    } else {
                        b.root_comments_deleted++;
                    }
                });
            } else if (o.op.which() ==
                       operation::tag<withdraw_vesting_operation>::value) {
                withdraw_vesting_operation op = o.op.get<withdraw_vesting_operation>();
                auto &account = db.get_account(op.account);
                const auto &bucket = db.get(bucket_id);

                auto new_vesting_withdrawal_rate =
                        op.vesting_shares.amount /
                        STEEMIT_VESTING_WITHDRAW_INTERVALS;
                if (op.vesting_shares.amount > 0 &&
                    new_vesting_withdrawal_rate == 0) {
                        new_vesting_withdrawal_rate = 1;
                }

                if (!db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
                    new_vesting_withdrawal_rate *= 10000;
                }

                db.modify(bucket, [&](bucket_object &b) {
                    if (account.vesting_withdraw_rate.amount > 0) {
                        b.modified_vesting_withdrawal_requests++;
                    } else {
                        b.new_vesting_withdrawal_requests++;
                    }

                    // TODO: Figure out how to change delta when a vesting withdraw finishes. Have until March 24th 2018 to figure that out...
                    b.vesting_withdraw_rate_delta +=
                            new_vesting_withdrawal_rate -
                            account.vesting_withdraw_rate.amount;
                });
            }
        }
    }

    void blockchain_statistics_plugin_impl::post_operation(const operation_notification &o) {
        try {
            auto &db = _self.database();

            for (auto bucket_id : _current_buckets) {
                const auto &bucket = db.get(bucket_id);

                if (!is_virtual_operation(o.op)) {
                    db.modify(bucket, [&](bucket_object &b) {
                        b.operations++;
                    });
                }
                o.op.visit(operation_process(_self, bucket, stat_sender_));
            }
        } FC_CAPTURE_AND_RETHROW()
    }

    std::vector < std::string > calculate_buckets_delta (const bucket_object & a, const bucket_object & b) {
        std::vector < std::string > result;
        if (a.seconds - b.seconds != 0 ) {
            result.push_back("seconds:" + std::to_string(a.seconds - b.seconds) + "|c");
        }
        if (a.blocks - b.blocks != 0 ) {
            result.push_back("blocks:" + std::to_string(a.blocks - b.blocks) + "|c");
        }
        if (a.bandwidth - b.bandwidth != 0 ) {
            result.push_back("bandwidth:" + std::to_string(a.bandwidth - b.bandwidth) + "|c");
        }
        if (a.operations - b.operations != 0 ) {
            result.push_back("operations:" + std::to_string(a.operations - b.operations) + "|c");
        }
        if (a.transactions - b.transactions != 0 ) {
            result.push_back("transactions:" + std::to_string(a.transactions - b.transactions) + "|c");
        }
        if (a.transfers - b.transfers != 0 ) {
            result.push_back("transfers:" + std::to_string(a.transfers - b.transfers) + "|c");
        }
        if (a.steem_transferred - b.steem_transferred != 0 ) {
            result.push_back("steem_transferred:" + std::string(a.steem_transferred - b.steem_transferred) + "|c");
        }
        if (a.sbd_transferred - b.sbd_transferred != 0 ) {
            result.push_back("sbd_transferred:" + std::string(a.sbd_transferred - b.sbd_transferred) + "|c");
        }
        if (a.sbd_paid_as_interest - b.sbd_paid_as_interest != 0 ) {
            result.push_back("sbd_paid_as_interest:" + std::string(a.sbd_paid_as_interest - b.sbd_paid_as_interest) + "|c");
        }
        if (a.paid_accounts_created - b.paid_accounts_created != 0 ) {
            result.push_back("paid_accounts_created:" + std::to_string(a.paid_accounts_created - b.paid_accounts_created) + "|c");
        }
        if (a.mined_accounts_created - b.mined_accounts_created != 0 ) {
            result.push_back("mined_accounts_created:" + std::to_string(a.mined_accounts_created - b.mined_accounts_created) + "|c");
        }
        if (a.root_comments - b.root_comments != 0 ) {
            result.push_back("root_comments:" + std::to_string(a.root_comments - b.root_comments) + "|c");
        }
        if (a.root_comment_edits - b.root_comment_edits != 0 ) {
            result.push_back("root_comment_edits:" + std::to_string(a.root_comment_edits - b.root_comment_edits) + "|c");
        }
        if (a.root_comments_deleted - b.root_comments_deleted != 0 ) {
            result.push_back("root_comments_deleted:" + std::to_string(a.root_comments_deleted - b.root_comments_deleted) + "|c");
        }
        if (a.replies - b.replies != 0 ) {
            result.push_back("replies:" + std::to_string(a.replies - b.replies) + "|c");
        }
        if (a.reply_edits - b.reply_edits != 0 ) {
            result.push_back("reply_edits:" + std::to_string(a.reply_edits - b.reply_edits) + "|c");
        }
        if (a.replies_deleted - b.replies_deleted != 0 ) {
            result.push_back("replies_deleted:" + std::to_string(a.replies_deleted - b.replies_deleted) + "|c");
        }
        if (a.new_root_votes - b.new_root_votes != 0 ) {
            result.push_back("new_root_votes:" + std::to_string(a.new_root_votes - b.new_root_votes) + "|c");
        }
        if (a.changed_root_votes - b.changed_root_votes != 0 ) {
            result.push_back("changed_root_votes:" + std::to_string(a.changed_root_votes - b.changed_root_votes) + "|c");
        }
        if (a.new_reply_votes - b.new_reply_votes != 0 ) {
            result.push_back("new_reply_votes:" + std::to_string(a.new_reply_votes - b.new_reply_votes) + "|c");
        }
        if (a.changed_reply_votes - b.changed_reply_votes != 0 ) {
            result.push_back("changed_reply_votes:" + std::to_string(a.changed_reply_votes - b.changed_reply_votes) + "|c");
        }
        if (a.payouts - b.payouts != 0 ) {
            result.push_back("payouts:" + std::to_string(a.payouts - b.payouts) + "|c");
        }
        if (a.sbd_paid_to_authors - b.sbd_paid_to_authors != 0 ) {
            result.push_back("sbd_paid_to_authors:" + std::string(a.sbd_paid_to_authors - b.sbd_paid_to_authors) + "|c");
        }
        if (a.vests_paid_to_authors - b.vests_paid_to_authors != 0 ) {
            result.push_back("vests_paid_to_authors:" + std::string(a.vests_paid_to_authors - b.vests_paid_to_authors) + "|c");
        }
        if (a.vests_paid_to_curators - b.vests_paid_to_curators != 0 ) {
            result.push_back("vests_paid_to_curators:" + std::string(a.vests_paid_to_curators - b.vests_paid_to_curators) + "|c");
        }
        if (a.liquidity_rewards_paid - b.liquidity_rewards_paid != 0 ) {
            result.push_back("liquidity_rewards_paid:" + std::string(a.liquidity_rewards_paid - b.liquidity_rewards_paid) + "|c");
        }
        if (a.transfers_to_vesting - b.transfers_to_vesting != 0 ) {
            result.push_back("transfers_to_vesting:" + std::to_string(a.transfers_to_vesting - b.transfers_to_vesting) + "|c");
        }
        if (a.steem_vested - b.steem_vested != 0 ) {
            result.push_back("steem_vested:" + std::string(a.steem_vested - b.steem_vested) + "|c");
        }
        if (a.new_vesting_withdrawal_requests - b.new_vesting_withdrawal_requests != 0 ) {
            result.push_back("new_vesting_withdrawal_requests:" + std::to_string(a.new_vesting_withdrawal_requests - b.new_vesting_withdrawal_requests) + "|c");
        }
        if (a.modified_vesting_withdrawal_requests - b.modified_vesting_withdrawal_requests != 0 ) {
            result.push_back("modified_vesting_withdrawal_requests:" + std::to_string(a.modified_vesting_withdrawal_requests - b.modified_vesting_withdrawal_requests) + "|c");
        }
        if (a.vesting_withdraw_rate_delta - b.vesting_withdraw_rate_delta != 0 ) {
            result.push_back("vesting_withdraw_rate_delta:" + std::string(a.vesting_withdraw_rate_delta - b.vesting_withdraw_rate_delta) + "|c");
        }
        if (a.vesting_withdrawals_processed - b.vesting_withdrawals_processed != 0 ) {
            result.push_back("vesting_withdrawals_processed:" + std::to_string(a.vesting_withdrawals_processed - b.vesting_withdrawals_processed) + "|c");
        }
        if (a.finished_vesting_withdrawals - b.finished_vesting_withdrawals != 0 ) {
            result.push_back("finished_vesting_withdrawals:" + std::to_string(a.finished_vesting_withdrawals - b.finished_vesting_withdrawals) + "|c");
        }
        if (a.vests_withdrawn - b.vests_withdrawn != 0 ) {
            result.push_back("vests_withdrawn:" + std::string(a.vests_withdrawn - b.vests_withdrawn) + "|c");
        }
        if (a.vests_transferred - b.vests_transferred != 0 ) {
            result.push_back("vests_transferred:" + std::string(a.vests_transferred - b.vests_transferred) + "|c");
        }
        if (a.sbd_conversion_requests_created - b.sbd_conversion_requests_created != 0 ) {
            result.push_back("sbd_conversion_requests_created:" + std::to_string(a.sbd_conversion_requests_created - b.sbd_conversion_requests_created) + "|c");
        }
        if (a.sbd_to_be_converted - b.sbd_to_be_converted != 0 ) {
            result.push_back("sbd_to_be_converted:" + std::string(a.sbd_to_be_converted - b.sbd_to_be_converted) + "|c");
        }
        if (a.sbd_conversion_requests_filled - b.sbd_conversion_requests_filled != 0 ) {
            result.push_back("sbd_conversion_requests_filled:" + std::to_string(a.sbd_conversion_requests_filled - b.sbd_conversion_requests_filled) + "|c");
        }
        if (a.steem_converted - b.steem_converted != 0 ) {
            result.push_back("steem_converted:" + std::string(a.steem_converted - b.steem_converted) + "|c");
        }
        if (a.limit_orders_created - b.limit_orders_created != 0 ) {
            result.push_back("limit_orders_created:" + std::to_string(a.limit_orders_created - b.limit_orders_created) + "|c");
        }
        if (a.limit_orders_filled - b.limit_orders_filled != 0 ) {
            result.push_back("limit_orders_filled:" + std::to_string(a.limit_orders_filled - b.limit_orders_filled) + "|c");
        }
        if (a.limit_orders_cancelled - b.limit_orders_cancelled != 0 ) {
            result.push_back("limit_orders_cancelled:" + std::to_string(a.limit_orders_cancelled - b.limit_orders_cancelled) + "|c");
        }
        if (a.total_pow - b.total_pow != 0 ) {
            result.push_back("total_pow:" + std::to_string(a.total_pow - b.total_pow) + "|c");
        }
        if (a.estimated_hashpower - b.estimated_hashpower != 0 ) {
            result.push_back("estimated_hashpower:" + std::string(a.estimated_hashpower - b.estimated_hashpower) + "|c");
        }
        return result;
    }

} // detail

blockchain_statistics_plugin::blockchain_statistics_plugin(application *app)
        : plugin(app),
          _my(new detail::blockchain_statistics_plugin_impl(*this)) {
}

blockchain_statistics_plugin::~blockchain_statistics_plugin() {
    _my->stat_sender_.reset();
    wlog("chain_stats plugin: stat_sender was shoutdowned");
}

void blockchain_statistics_plugin::plugin_set_program_options(
        boost::program_options::options_description &cli,
        boost::program_options::options_description &cfg
) {
    cli.add_options()
            ("chain-stats-bucket-size", boost::program_options::value<string>()->default_value("[60,3600,21600,86400,604800,2592000]"),
                    "Track blockchain statistics by grouping orders into buckets of equal size measured in seconds specified as a JSON array of numbers")
            ("chain-stats-history-per-bucket", boost::program_options::value<uint32_t>()->default_value(100),
                    "How far back in time to track history for each bucket size, measured in the number of buckets (default: 100)");
    cfg.add(cli);
}

void blockchain_statistics_plugin::plugin_initialize(const boost::program_options::variables_map &options) {

    try {
        ilog("chain_stats_plugin: plugin_initialize() begin");

        uint32_t statistics_sender_default_port;

        if (options.count("statistics_sender_default_port")) {
            statistics_sender_default_port = options["statistics_sender_default_port"].as<uint32_t>();
        }

        _my->stat_sender_ = std::shared_ptr<statistics_sender>(new statistics_sender(statistics_sender_default_port) );

        wlog("chain_stats plugin: statistics_sender was initialized");

        chain::database &db = database();

        db.applied_block.connect([&](const signed_block &b) { _my->on_block(b); });
        db.pre_apply_operation.connect([&](const operation_notification &o) { _my->pre_operation(o); });
        db.post_apply_operation.connect([&](const operation_notification &o) { _my->post_operation(o); });

        add_plugin_index<bucket_index>(db);

        if (options.count("chain-stats-bucket-size")) {
            const std::string &buckets = options["chain-stats-bucket-size"].as<string>();
            _my->_tracked_buckets = fc::json::from_string(buckets).as<flat_set<uint32_t>>();
        }
        if (options.count("chain-stats-history-per-bucket")) {
            _my->_maximum_history_per_bucket_size = options["chain-stats-history-per-bucket"].as<uint32_t>();
        }
        if (options.count("chain-stats-statsd-ip")) {
            for (auto it : options["chain-stats-statsd-ip"].as<std::vector<std::string>>()) {
                _my->stat_sender_->add_address(it);
            }
        }

        wlog("chain-stats-bucket-size: ${b}", ("b", _my->_tracked_buckets));
        wlog("chain-stats-history-per-bucket: ${h}", ("h", _my->_maximum_history_per_bucket_size));

        ilog("chain_stats_plugin: plugin_initialize() end");

        // const auto &bucket_idx = db.get_index<bucket_index>().indices().get<by_id>();

    

    } FC_CAPTURE_AND_RETHROW()
}

void blockchain_statistics_plugin::plugin_startup() {
    ilog("chain_stats plugin: plugin_startup() begin");

    app().register_api_factory<blockchain_statistics_api>("chain_stats_api");

    if ( _my->stat_sender_->can_start() ) {
        _my->stat_sender_->start_sending();
        wlog("chain_stats plugin: statistics_sender was started");
        wlog("recipients endpoints: ${endpoints}", ( "endpoints", _my->stat_sender_->get_endpoint_string_vector() ) );
    }
    else {
        wlog("chain_stats plugin: statistics_sender was not started: no recipient's IPs were provided.");
    }

    ilog("chain_stats plugin: plugin_startup() end");
}

const flat_set<uint32_t> &blockchain_statistics_plugin::get_tracked_buckets() const {
    return _my->_tracked_buckets;
}

uint32_t blockchain_statistics_plugin::get_max_history_per_bucket() const {
    return _my->_maximum_history_per_bucket_size;
}

}
} // steemit::blockchain_statistics

STEEMIT_DEFINE_PLUGIN(blockchain_statistics, steemit::blockchain_statistics::blockchain_statistics_plugin);
