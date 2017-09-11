#include <steemit/plugins/database_api/database_api.hpp>

#include <steemit/plugins/follow_api/follow_api.hpp>
#include <steemit/plugins/snapshot/snapshot_plugin.hpp>
#include <steemit/plugins/market_history/market_history_plugin.hpp>
#include <steemit/plugins/follow/follow_plugin.hpp>
#include <steemit/plugins/languages/languages_plugin.hpp>
#include <steemit/plugins/tags/tags_plugin.hpp>

#include <steemit/chain/utilities/reward.hpp>
#include <steemit/protocol/get_config.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>

#define GET_REQUIRED_FEES_MAX_RECURSION 4

#define CHECK_ARG_SIZE( s ) \
   FC_ASSERT( args.size() == s, "Expected #s argument(s), was ${n}", ("n", args.size()) );

namespace steemit {
    namespace plugins {
        namespace database_api {

        struct database_api::database_api_impl {
        public:
            database_api_impl();

            ~database_api_impl();

            // Subscriptions
            void set_subscribe_callback(std::function<void(const variant &)> cb, bool clear_filter);

            void set_pending_transaction_callback(std::function<void(const variant &)> cb);

            void set_block_applied_callback(std::function<void(const variant &block_id)> cb);

            void cancel_all_subscriptions();
            // Blocks and transactions
            optional<block_header> get_block_header(uint32_t block_num) const;

            optional<signed_block> get_block(uint32_t block_num) const;

            std::vector<applied_operation> get_ops_in_block(uint32_t block_num, bool only_virtual) const;

            // Globals
            fc::variant_object get_config() const;

            dynamic_global_property_api_object get_dynamic_global_properties() const;

            // Accounts
            std::vector<extended_account> get_accounts(std::vector<std::string> names) const;

            std::vector<optional<account_api_object>> lookup_account_names(const std::vector<std::string> &account_names) const;

            std::set<std::string> lookup_accounts(const std::string &lower_bound_name, uint32_t limit) const;

            uint64_t get_account_count() const;

            // Witnesses
            std::vector<optional<witness_api_object>> get_witnesses(const std::vector<witness_object::id_type> &witness_ids) const;

            fc::optional<witness_api_object> get_witness_by_account(std::string account_name) const;

            std::set<account_name_type> lookup_witness_accounts(const std::string &lower_bound_name, uint32_t limit) const;

            uint64_t get_witness_count() const;

            // Balances
            vector<asset> get_account_balances(account_name_type account_name, const flat_set<asset_name_type> &assets) const;

            // Assets
            vector<optional<asset_object>> get_assets(const vector<asset_name_type> &asset_symbols) const;

            vector<optional<asset_object>> get_assets_by_issuer(string issuer) const;

            vector<optional<asset_dynamic_data_object>> get_assets_dynamic_data(const vector<asset_name_type> &asset_symbols) const;

            vector<optional<asset_bitasset_data_object>> get_bitassets_data(const vector<asset_name_type> &asset_symbols) const;

            vector<asset_object> list_assets(const asset_name_type &lower_bound_symbol, uint32_t limit) const;

            vector<optional<asset_object>> lookup_asset_symbols(const vector<asset_name_type> &asset_symbols) const;

            // Authority / validation
            std::string get_transaction_hex(const signed_transaction &trx) const;

            std::set<public_key_type> get_required_signatures(const signed_transaction &trx, const flat_set<public_key_type> &available_keys) const;

            std::set<public_key_type> get_potential_signatures(const signed_transaction &trx) const;

            bool verify_authority(const signed_transaction &trx) const;

            bool verify_account_authority(const std::string &name_or_id, const flat_set<public_key_type> &signers) const;

            // Proposed transactions
            vector<proposal_object> get_proposed_transactions(account_name_type name) const;

            template<typename T>
            void subscribe_to_item(const T &i) const {
                auto vec = fc::raw::pack(i);
                if (!_subscribe_callback) {
                    return;
                }

                if (!is_subscribed_to_item(i)) {
                    idump((i));
                    _subscribe_filter.insert(vec.data(), vec.size());//(vecconst char*)&i, sizeof(i) );
                }
            }

            template<typename T>
            bool is_subscribed_to_item(const T &i) const {
                if (!_subscribe_callback) {
                    return false;
                }

                return _subscribe_filter.contains(i);
            }

            // signal handlers
            void on_applied_block(const chain::signed_block &b);

            mutable fc::bloom_filter _subscribe_filter;
            std::function<void(const fc::variant &)> _subscribe_callback;
            std::function<void(const fc::variant &)> _pending_trx_callback;
            std::function<void(const fc::variant &)> _block_applied_callback;


            steemit::chain::database & database() {
                return _db;
            }
            std::shared_ptr<plugins::follow::follow_api> _follow_api;

            boost::signals2::scoped_connection _block_applied_connection;

            map<pair<asset_symbol_type, asset_symbol_type>, std::function<void(const variant &)>> _market_subscriptions;
        private:

            steemit::chain::database &_db;
        };



        void find_accounts(std::set<std::string> &accounts, const discussion &d) {
            accounts.insert(d.author);
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Subscriptions                                                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        void database_api::database_api::set_subscribe_callback(std::function<void(const variant &)> cb, bool clear_filter) {
            my->database().with_read_lock([&]() {
                my->set_subscribe_callback(cb, clear_filter);
            });
        }

        void database_api::database_api_impl::set_subscribe_callback(std::function<void(const variant &)> cb, bool clear_filter) {
            _subscribe_callback = cb;
            if (clear_filter || !cb) {
                static fc::bloom_parameters param;
                param.projected_element_count = 10000;
                param.false_positive_probability = 1.0 / 10000;
                param.maximum_size = 1024 * 8 * 8 * 2;
                param.compute_optimal_parameters();
                _subscribe_filter = fc::bloom_filter(param);
            }
        }

        void database_api::database_api::set_pending_transaction_callback(std::function<void(const variant &)> cb) {
            my->database().with_read_lock([&]() {
                my->set_pending_transaction_callback(cb);
            });
        }

        void database_api::database_api_impl::set_pending_transaction_callback(std::function<void(const variant &)> cb) {
            _pending_trx_callback = cb;
        }

        void database_api::database_api_impl::set_block_applied_callback(std::function<void(const variant &block_id)> cb) {
            database().with_read_lock([&]() {
                set_block_applied_callback(cb);
            });
        }

        void database_api::database_api_impl::on_applied_block(const chain::signed_block &b) {
            try {
                _block_applied_callback(fc::variant(signed_block_header(b)));
            }
            catch (...) {
                _block_applied_connection.release();
            }
        }

        void database_api::database_api_impl::set_block_applied_callback(std::function<void(const variant &block_header)> cb) {
            _block_applied_callback = cb;
            _block_applied_connection = connect_signal(database().applied_block, *this, &database_api::database_api_impl::on_applied_block);
        }

        void database_api::cancel_all_subscriptions() {
            my->database().with_read_lock([&]() {
                my->cancel_all_subscriptions();
            });
        }

        void database_api::database_api_impl::cancel_all_subscriptions() {
            set_subscribe_callback(std::function<void(const fc::variant &)>(), true);
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Constructors                                                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        database_api::database_api()
                : my(new database_api_impl()) {
                JSON_RPC_REGISTER_API(
                    __name__,
                    (get_trending_tags)
                    (get_state)
                    (get_trending_categories)
                    (get_best_categories)
                    (get_active_categories)
                    (get_recent_categories)
                    (get_active_witnesses)
                    (get_miner_queue)
                    (get_block_header)
                    (get_block)
                    (get_ops_in_block)
                    (get_config)
                    (get_dynamic_global_properties)
                    (get_chain_properties)
                    (get_current_median_history_price)
                    (get_feed_history)
                    (get_witness_schedule)
                    (get_hardfork_version)
                    (get_next_scheduled_hardfork)
                    (get_reward_fund)
                    (get_accounts)
                    (lookup_account_names)
                    (lookup_accounts)
                    (get_account_balances)
                    (get_account_count)
                    (get_owner_history)
                    (get_recovery_request)
                    (get_escrow)
                    (get_withdraw_routes)
                    (get_account_bandwidth)
                    (get_savings_withdraw_from)
                    (get_savings_withdraw_to)
                    (get_vesting_delegations)
                    (get_expiring_vesting_delegations)
                    (get_witnesses)
                    (get_conversion_requests)
                    (get_witness_by_account)
                    (get_witnesses_by_vote)
                    (lookup_witness_accounts)
                    (get_witness_count)
                    (get_assets)
                    (get_assets_by_issuer)
                    (get_assets_dynamic_data)
                    (get_bitassets_data)
                    (list_assets)
                    (lookup_asset_symbols)
                    (get_transaction_hex)
                    (get_transaction)
                    (get_required_signatures)
                    (get_potential_signatures)
                    (verify_authority)
                    (verify_account_authority)
                    (get_active_votes)
                    (get_account_votes)
                    (get_content)
                    (get_content_replies)
                    (get_tags_used_by_author)
                    (get_discussions_by_trending)
                    (get_discussions_by_created)
                    (get_discussions_by_active)
                    (get_discussions_by_cashout)
                    (get_discussions_by_payout)
                    (get_post_discussions_by_payout)
                    (get_comment_discussions_by_payout)
                    (get_discussions_by_votes)
                    (get_discussions_by_children)
                    (get_discussions_by_hot)
                    (get_discussions_by_feed)
                    (get_discussions_by_blog)
                    (get_discussions_by_comments)
                    (get_discussions_by_promoted)
                    (get_replies_by_last_update)
                    (get_discussions_by_author_before_date)
                    (get_account_history)
                    (get_payout_extension_cost)
                    (get_payout_extension_time)
                    (get_proposed_transactions)
            )
        }

        database_api::~database_api() {
        }

            database_api::database_api_impl::database_api_impl()
                    :_db( appbase::app().get_plugin< steemit::plugins::chain::chain_plugin >().db() ){
            wlog("creating database api ${x}", ("x", int64_t(this)));

            try {
                appbase::app().get_plugin<follow::follow_plugin>();
                _follow_api = std::make_shared<follow_api::follow_api>();
            }
            catch (fc::assert_exception) {
                ilog("Follow Plugin not loaded");
            }

#ifndef STEEMIT_BUILD_TESTNET
            try {
                FC_ASSERT(appbase::app().get_plugin<snapshot::snapshot_plugin>().get_loaded_snapshots().right.at("1f0617dfc2e7aa49b0d6c394b36087ead02bc7f781e7550dae13e8cb12f13436").find("snapshot5392323.json"), "Steemit accounts snapsnot is invalid");

            } catch (fc::assert_exception) {
                elog("Snapshots plugin not loaded");
            }
#endif
        }

        database_api::database_api_impl::~database_api_impl() {
            elog("freeing database api ${x}", ("x", int64_t(this)));
        }


//////////////////////////////////////////////////////////////////////
//                                                                  //
// Blocks and transactions                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        DEFINE_API(database_api,get_block_header){
            CHECK_ARG_SIZE( 1 )
            return my->database().with_read_lock([&]() {
                return my->get_block_header(args[0].as< uint32_t >());
            });
        }

        optional<block_header> database_api::database_api_impl::get_block_header(uint32_t block_num) const {
                auto result = _db.fetch_block_by_number(block_num);
                if (result) {
                    return *result;
                }
                return {};
        }

        DEFINE_API(database_api,get_block){
            CHECK_ARG_SIZE( 1 )
            return my->database().with_read_lock([&]() {
                return my->get_block(args[0].as< uint32_t >());
            });
        }

        optional<signed_block> database_api::database_api_impl::get_block(uint32_t block_num) const {
                return my->database().fetch_block_by_number(block_num);
        }

        DEFINE_API(database_api,get_ops_in_block){
            CHECK_ARG_SIZE( 2 )
            return my->database().with_read_lock([&]() {
                return my->get_ops_in_block(args[0].as< uint32_t >(), args[1].as< bool >() );
            });
        }

        std::vector<applied_operation> database_api::database_api_impl::get_ops_in_block(uint32_t block_num, bool only_virtual) const {
                const auto &idx = my->database().get_index<operation_index>().indices().get<by_location>();
                auto itr = idx.lower_bound(block_num);
                std::vector<applied_operation> result;
                applied_operation temp;
                while (itr != idx.end() && itr->block == block_num) {
                    temp = *itr;
                    if (!only_virtual || is_virtual_operation(temp.op)) {
                        result.push_back(temp);
                    }
                    ++itr;
                }
                return result;
        }


//////////////////////////////////////////////////////////////////////
//                                                                  //
// Globals                                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        DEFINE_API(database_api,get_config) {
            return my->database().with_read_lock(
                    [&]() {
                        return my->get_config();
                    }
            );
        }

        fc::variant_object database_api::database_api_impl::get_config() const {
                return steemit::protocol::get_config();
        }

        DEFINE_API(database_api,get_dynamic_global_properties){
            return my->database().with_read_lock(
                    [&]() {
                        return my->get_dynamic_global_properties();
                    }
            );
        }

        DEFINE_API(database_api,get_chain_properties){
            return my->database().with_read_lock(
                    [&]() {
                        return my->database().get_witness_schedule_object().median_props;
                    }
            );
        }

        DEFINE_API(database_api,get_feed_history){
            return my->database().with_read_lock(
                    [&]() {
                        return feed_history_api_obj(my->database().get_feed_history());
                    }
            );
        }

        DEFINE_API(database_api,get_current_median_history_price){
            return my->database().with_read_lock(
                    [&]() {
                        return my->database().get_feed_history().current_median_history;
                    }
            );
        }

        dynamic_global_property_api_object database_api::database_api_impl::get_dynamic_global_properties() const {
                return my->database().get(dynamic_global_property_object::id_type());
        }

        DEFINE_API(database_api,get_witness_schedule){
            return my->database().with_read_lock(
                    [&]() {
                        return my->database().get(witness_schedule_object::id_type());
                    }
            );
        }

        DEFINE_API(database_api,get_hardfork_version){
            return my->database().with_read_lock([&]() {
                return my->database().get(hardfork_property_object::id_type()).current_hardfork_version;
            });
        }

        DEFINE_API(database_api,get_next_scheduled_hardfork){
            return my->database().with_read_lock([&]() {
                scheduled_hardfork shf;
                const auto &hpo = my->database().get(hardfork_property_object::id_type());
                shf.hf_version = hpo.next_hardfork;
                shf.live_time = hpo.next_hardfork_time;
                return shf;
            });
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Accounts                                                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        DEFINE_API(database_api,get_accounts){
            CHECK_ARG_SIZE( 1 )
            return my->database().with_read_lock([&]() {
                return my->get_accounts(args[0].as< vector< std::string > >());
            });
        }

        std::vector<extended_account> database_api::database_api_impl::get_accounts(std::vector<std::string> names) const {
                const auto &idx = my->database().get_index<account_index>().indices().get<by_name>();
                const auto &vidx = my->database().get_index<witness_vote_index>().indices().get<by_account_witness>();
                std::vector<extended_account> results;

                for (auto name: names) {
                    auto itr = idx.find(name);
                    if (itr != idx.end()) {
                        results.push_back(extended_account(*itr, _db));

                        if (_follow_api) {
                            results.back().reputation = _follow_api->get_account_reputations(itr->name, 1)[0].reputation;
                        }

                        auto vitr = vidx.lower_bound(boost::make_tuple(itr->id, witness_object::id_type()));
                        while (vitr != vidx.end() && vitr->account == itr->id) {
                            results.back().witness_votes.insert(_db.get(vitr->witness).owner);
                            ++vitr;
                        }
                    }
                }

                return results;
        }

        DEFINE_API(database_api,lookup_account_names){
            CHECK_ARG_SIZE( 1 )
            return my->database().with_read_lock([&]() {
                return my->lookup_account_names(args[0].as< vector< std::string > >());
            });
        }

        std::vector<optional<account_api_object>> database_api::database_api_impl::lookup_account_names(const std::vector<std::string> &account_names) const {
                std::vector<optional<account_api_object>> result;
                result.reserve(account_names.size());

                for (auto &name : account_names) {
                    auto itr = _db.find<account_object, by_name>(name);

                    if (itr) {
                        result.push_back(account_api_object(*itr, database()));
                    } else {
                        result.push_back(optional<account_api_object>());
                    }
                }

                return result;
        }

        DEFINE_API(database_api,lookup_accounts){
            CHECK_ARG_SIZE( 2 )
            account_name_type lower_bound_name = args[0].as< account_name_type >();
            uint32_t limit = args[1].as< uint32_t >();
            return my->database().with_read_lock([&]() {
                return my->lookup_accounts(lower_bound_name, limit);
            });
        }

        std::set<std::string> database_api::database_api_impl::lookup_accounts(const std::string &lower_bound_name, uint32_t limit) const {
                FC_ASSERT(limit <= 1000);
                const auto &accounts_by_name = my->database().get_index<account_index>().indices().get<by_name>();
                std::set<std::string> result;

                for (auto itr = accounts_by_name.lower_bound(lower_bound_name); limit-- && itr != accounts_by_name.end(); ++itr) {
                    result.insert(itr->name);
                }

                return result;
        }

        DEFINE_API(database_api,get_account_count){
            return my->database().with_read_lock([&]() {
                return my->get_account_count();
            });
        }

        uint64_t database_api::database_api_impl::get_account_count() const {
            return database().get_index<account_index>().indices().size();
        }

        DEFINE_API(database_api,get_owner_history){
            CHECK_ARG_SIZE( 1 )
            auto account=args[0].as< string >();
            return my->database().with_read_lock([&]() {
                std::vector<owner_authority_history_api_object> results;
                const auto &hist_idx = my->database().get_index<owner_authority_history_index>().indices().get<by_account>();
                auto itr = hist_idx.lower_bound(account);

                while (itr != hist_idx.end() && itr->account == account) {
                    results.push_back(owner_authority_history_api_object(*itr));
                    ++itr;
                }

                return results;
            });
        }

        DEFINE_API(database_api,get_recovery_request){
            CHECK_ARG_SIZE( 1 )
            auto account = args[0].as< account_name_type >();
            return my->database().with_read_lock([&]() {
                optional<account_recovery_request_api_obj> result;

                const auto &rec_idx = my->database().get_index<account_recovery_request_index>().indices().get<by_account>();
                auto req = rec_idx.find(account);

                if (req != rec_idx.end()) {
                    result = account_recovery_request_api_obj(*req);
                }

                return result;
            });
        }

        DEFINE_API(database_api,get_escrow){
            CHECK_ARG_SIZE( 2 )
            auto from = args[0].as< account_name_type >();
            auto escrow_id = args[1].as< uint32_t >() ;
            return my->database().with_read_lock([&]() {
                optional<escrow_api_object> result;

                try {
                    result = my->database().get_escrow(from, escrow_id);
                }
                catch (...) {
                }

                return result;
            });
        }

        DEFINE_API(database_api,get_withdraw_routes) {
            FC_ASSERT( args.size() == 1 || args.size() == 2, "Expected 1-2 arguments, was ${n}", ("n", args.size()) );
            auto account = args[0].as< string >();
            auto type = args.size() == args[1].as< withdraw_route_type >();

            return my->database().with_read_lock([&]() {
                std::vector<withdraw_route> result;

                const auto &acc = my->database().get_account(account);

                if (type == outgoing || type == all) {
                    const auto &by_route = my->database().get_index<withdraw_vesting_route_index>().indices().get<by_withdraw_route>();
                    auto route = by_route.lower_bound(acc.id);

                    while (route != by_route.end() && route->from_account == acc.id) {
                        withdraw_route r;
                        r.from_account = account;
                        r.to_account = my->database().get(route->to_account).name;
                        r.percent = route->percent;
                        r.auto_vest = route->auto_vest;

                        result.push_back(r);

                        ++route;
                    }
                }

                if (type == incoming || type == all) {
                    const auto &by_dest = my->database().get_index<withdraw_vesting_route_index>().indices().get<by_destination>();
                    auto route = by_dest.lower_bound(acc.id);

                    while (route != by_dest.end() && route->to_account == acc.id) {
                        withdraw_route r;
                        r.from_account = my->database().get(route->from_account).name;
                        r.to_account = account;
                        r.percent = route->percent;
                        r.auto_vest = route->auto_vest;

                        result.push_back(r);

                        ++route;
                    }
                }

                return result;
            });
        }

        DEFINE_API(database_api,get_account_bandwidth){
            CHECK_ARG_SIZE( 2 )
            auto account = args[0].as< string >();
            auto type    = args[1].as< bandwidth_type >();
            optional<account_bandwidth_api_object> result;
            auto band = my->database().find<account_bandwidth_object, by_account_bandwidth_type>(boost::make_tuple(account, type));
            if (band != nullptr) {
                result = *band;
            }

            return result;
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Witnesses                                                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        DEFINE_API(database_api,get_witnesses) {
            return my->database().with_read_lock(
                    [&]() {
                        return my->get_witnesses(args);
                    }
            );
        }

        DEFINE_API(database_api::database_api_impl,get_witnesses){
            CHECK_ARG_SIZE( 1 )
            vector< witness_id_type > witness_ids = args[0].as< vector< witness_id_type > >();

            vector< optional< witness_api_object > > result;
            result.reserve( witness_ids.size() );
            result.reserve(witness_ids.size());
            std::transform(witness_ids.begin(), witness_ids.end(), std::back_inserter(result),
                    [this](witness_object::id_type id) -> optional<witness_api_object> {
                        if (auto o = _db.find(id)) {
                            return *o;
                        }
                        return {};
                    });
            return result;
        }

        DEFINE_API(database_api,get_witness_by_account){
            return my->database().with_read_lock([&]() {
                return my->get_witness_by_account(args);
            });
        }

        DEFINE_API(database_api,get_witnesses_by_vote){
            return my->database().with_read_lock([&]() {
                //idump((from)(limit));
                FC_ASSERT(limit <= 100);

                std::vector<witness_api_object> result;
                result.reserve(limit);

                const auto &name_idx = my->database().get_index<witness_index>().indices().get<by_name>();
                const auto &vote_idx = my->database().get_index<witness_index>().indices().get<by_vote_name>();

                auto itr = vote_idx.begin();
                if (from.size()) {
                    auto nameitr = name_idx.find(from);
                    FC_ASSERT(nameitr != name_idx.end(), "invalid witness name ${n}", ("n", from));
                    itr = vote_idx.iterator_to(*nameitr);
                }

                while (itr != vote_idx.end() && result.size() < limit && itr->votes > 0) {
                    result.push_back(witness_api_object(*itr));
                    ++itr;
                }
                return result;
            });
        }

        DEFINE_API(database_api::database_api_impl,get_witness_by_account){
            const auto &idx = database().get_index<witness_index>().indices().get<by_name>();
            auto itr = idx.find(account_name);
            if (itr != idx.end()) {
                return witness_api_object(*itr);
            }
            return {};
        }

        DEFINE_API(database_api,lookup_witness_accounts){
            return my->database().with_read_lock(
                    [&]() {
                        return my->lookup_witness_accounts(lower_bound_name, limit);
                    }
            );
        }

        DEFINE_API(database_api::database_api_impl,lookup_witness_accounts){
            FC_ASSERT(limit <= 1000);
            const auto &witnesses_by_id = database().get_index<witness_index>().indices().get<by_id>();

            // get all the names and look them all up, sort them, then figure out what
            // records to return.  This could be optimized, but we expect the
            // number of witnesses to be few and the frequency of calls to be rare
            std::set<account_name_type> witnesses_by_account_name;
            for (const witness_api_object &witness : witnesses_by_id) {
                if (witness.owner >= lower_bound_name) { // we can ignore anything below lower_bound_name
                    witnesses_by_account_name.insert(witness.owner);
                }
            }

            auto end_iter = witnesses_by_account_name.begin();
            while (end_iter != witnesses_by_account_name.end() && limit--) {
                ++end_iter;
            }
            witnesses_by_account_name.erase(end_iter, witnesses_by_account_name.end());
            return witnesses_by_account_name;
        }

        DEFINE_API(database_api,get_witness_count){
            return my->database().with_read_lock(
                    [&]() {
                        return my->get_witness_count();
                    }
            );
        }

        DEFINE_API(database_api::database_api_impl,get_witness_count){
            return database().get_index<witness_index>().indices().size();
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Balances                                                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        DEFINE_API(database_api,get_account_balances){
            return my->get_account_balances(name, assets);
        }

        DEFINE_API(database_api::database_api_impl,get_account_balances){
            vector<asset> result;
            if (assets.empty()) {
                // if the caller passes in an empty list of assets, return balances for all assets the account owns
                auto range = _db.get_index<account_balance_index>().indices().get<by_account_asset>().equal_range(boost::make_tuple(acnt));
                for (const account_balance_object &balance : boost::make_iterator_range(range.first, range.second)) {
                    result.push_back(asset(balance.get_balance()));
                }
            } else {
                result.reserve(assets.size());

                std::transform(assets.begin(), assets.end(), std::back_inserter(result),
                        [this, acnt](asset_name_type id) {
                            return _db.get_balance(acnt, id);
                        });
            }

            return result;
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Assets                                                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        DEFINE_API(database_api,get_assets){
            return my->get_assets(asset_symbols);
        }

        DEFINE_API(database_api::database_api_impl,get_assets){
            vector<optional<asset_object>> result;

            const auto &idx = database().get_index<asset_index>().indices().get<by_asset_name>();
            std::transform(asset_symbols.begin(), asset_symbols.end(), std::back_inserter(result),
                    [&](asset_name_type id) -> optional<asset_object> {
                        auto itr = idx.find(id);
                        if (itr != idx.end()) {
                            subscribe_to_item(id);
                            return *itr;
                        }
                        return {};
                    });
            return result;
        }

        DEFINE_API(database_api,get_assets_by_issuer){
            return my->get_assets_by_issuer(issuer);
        }

        DEFINE_API(database_api::database_api_impl,get_assets_by_issuer){
            vector<optional<asset_object>> result;

            auto range = database().get_index<asset_index>().indices().get<by_issuer>().equal_range(issuer);
            for (const asset_object &asset : boost::make_iterator_range(range.first, range.second)) {
                result.emplace_back(asset);
            }

            return result;
        }

        DEFINE_API(database_api,get_assets_dynamic_data){
            return my->get_assets_dynamic_data(asset_symbols);
        }

        DEFINE_API(database_api::database_api_impl::get_assets_dynamic_data){
            vector<optional<asset_dynamic_data_object>> result;

            const auto &idx = database().get_index<asset_dynamic_data_index>().indices().get<by_asset_name>();
            std::transform(asset_symbols.begin(), asset_symbols.end(), std::back_inserter(result),
                    [&](string symbol) -> optional<asset_dynamic_data_object> {
                        auto itr = idx.find(symbol);
                        if (itr != idx.end()) {
                            subscribe_to_item(symbol);
                            return *itr;
                        }
                        return {};
                    });
            return result;
        }

        DEFINE_API(database_api,get_bitassets_data){
            return my->get_bitassets_data(asset_symbols);
        }

        DEFINE_API(database_api::database_api_impl,get_bitassets_data){
            vector<optional<asset_bitasset_data_object>> result;

            const auto &idx = database().get_index<asset_bitasset_data_index>().indices().get<by_asset_name>();
            std::transform(asset_symbols.begin(), asset_symbols.end(), std::back_inserter(result),
                    [&](string symbol) -> optional<asset_bitasset_data_object> {
                        auto itr = idx.find(symbol);
                        if (itr != idx.end()) {
                            subscribe_to_item(symbol);
                            return *itr;
                        }
                        return {};
                    });
            return result;
        }

        DEFINE_API(database_api::database_api,list_assets){
            return my->list_assets(lower_bound_symbol, limit);
        }

        DEFINE_API(database_api::database_api_impl,list_assets){
            FC_ASSERT(limit <= 100);
            const auto &assets_by_symbol = database().get_index<asset_index>().indices().get<by_asset_name>();
            vector<asset_object> result;

            auto itr = assets_by_symbol.begin();

            if (lower_bound_symbol != "") {
                itr = assets_by_symbol.lower_bound(lower_bound_symbol);
            }

            while (limit-- && itr != assets_by_symbol.end()) {
                result.emplace_back(*itr++);
            }

            return result;
        }

        DEFINE_API(database_api,lookup_asset_symbols){
            return my->lookup_asset_symbols(asset_symbols);
        }

        DEFINE_API(database_api::database_api_impl,lookup_asset_symbols) {
            const auto &assets_by_symbol = database().get_index<asset_index>().indices().get<by_asset_name>();
            vector<optional<asset_object>> result;
            result.reserve(asset_symbols.size());
            std::transform(asset_symbols.begin(), asset_symbols.end(), std::back_inserter(result),
                    [this, &assets_by_symbol](const vector<asset_name_type>::value_type &symbol) -> optional<asset_object> {
                        auto ptr = database().find_asset(symbol);
                        return ptr == nullptr ? optional<asset_object>() : *ptr;
                    });
            return result;
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Authority / validation                                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////

        DEFINE_API(database_api,get_transaction_hex){
            return my->database().with_read_lock(
                    [&]() {
                        return my->get_transaction_hex(trx);
                    }
            );
        }

        DEFINE_API(database_api::database_api_impl,get_transaction_hex){
            return fc::to_hex(fc::raw::pack(trx));
        }

        DEFINE_API(database_api,get_required_signatures){
                return my->database().with_read_lock([&]() {
                    return my->get_required_signatures(trx, available_keys);
                });
        }

        DEFINE_API(database_api::database_api_impl,get_required_signatures){
//   wdump((trx)(available_keys));
            auto result = trx.get_required_signatures(STEEMIT_CHAIN_ID,
                    available_keys,
                    [&](std::string account_name) {
                        return authority(database().get<account_authority_object, by_account>(account_name).active);
                    },
                    [&](std::string account_name) {
                        return authority(database().get<account_authority_object, by_account>(account_name).owner);
                    },
                    [&](std::string account_name) {
                        return authority(database().get<account_authority_object, by_account>(account_name).posting);
                    },
                    STEEMIT_MAX_SIG_CHECK_DEPTH);
//   wdump((result));
            return result;
        }

        DEFINE_API(database_api,get_potential_signatures){
            return my->database().with_read_lock([&]() {
                return my->get_potential_signatures(trx);
            });
        }

        DEFINE_API(database_api::database_api_impl,get_potential_signatures){
//   wdump((trx));
            std::set<public_key_type> result;
            trx.get_required_signatures(
                    STEEMIT_CHAIN_ID,
                    flat_set<public_key_type>(),
                    [&](account_name_type account_name) {
                        const auto &auth = database().get<account_authority_object, by_account>(account_name).active;
                        for (const auto &k : auth.get_keys()) {
                            result.insert(k);
                        }
                        return authority(auth);
                    },
                    [&](account_name_type account_name) {
                        const auto &auth = database().get<account_authority_object, by_account>(account_name).owner;
                        for (const auto &k : auth.get_keys()) {
                            result.insert(k);
                        }
                        return authority(auth);
                    },
                    [&](account_name_type account_name) {
                        const auto &auth = database().get<account_authority_object, by_account>(account_name).posting;
                        for (const auto &k : auth.get_keys()) {
                            result.insert(k);
                        }
                        return authority(auth);
                    },
                    STEEMIT_MAX_SIG_CHECK_DEPTH
            );

//   wdump((result));
            return result;
        }

        DEFINE_API(database_api,verify_authority){
            return my->database().with_read_lock([&]() {
                return my->verify_authority(trx);
            });
        }

        DEFINE_API(database_api::database_api_impl,verify_authority){
            trx.verify_authority(STEEMIT_CHAIN_ID,
                    [&](std::string account_name) {
                        return authority(database().get<account_authority_object, by_account>(account_name).active);
                    },
                    [&](std::string account_name) {
                        return authority(database().get<account_authority_object, by_account>(account_name).owner);
                    },
                    [&](std::string account_name) {
                        return authority(database().get<account_authority_object, by_account>(account_name).posting);
                    },
                    STEEMIT_MAX_SIG_CHECK_DEPTH);
            return true;
        }

        DEFINE_API(database_api,verify_account_authority){
            return my->database().with_read_lock([&]() {
                return my->verify_account_authority(name, signers);
            });
        }

        DEFINE_API(database_api::database_api_impl,verify_account_authority){
            FC_ASSERT(name.size() > 0);
            auto account = database().find<account_object, by_name>(name);
            FC_ASSERT(account, "no such account");

            /// reuse trx.verify_authority by creating a dummy transfer
            signed_transaction trx;
            transfer_operation op;
            op.from = account->name;
            trx.operations.emplace_back(op);

            return verify_authority(trx);
        }

        DEFINE_API(database_api,get_conversion_requests){
            return my->database().with_read_lock([&]() {
                const auto &idx = my->database().get_index<convert_request_index>().indices().get<by_owner>();
                std::vector<convert_request_api_object> result;
                auto itr = idx.lower_bound(account);
                while (itr != idx.end() && itr->owner == account) {
                    result.emplace_back(*itr);
                    ++itr;
                }
                return result;
            });
        }

        DEFINE_API(database_api,get_content){
            return my->database().with_read_lock([&]() {
                const auto &by_permlink_idx = my->_db.get_index<comment_index>().indices().get<by_permlink>();
                auto itr = by_permlink_idx.find(boost::make_tuple(author, permlink));
                if (itr != by_permlink_idx.end()) {
                    discussion result(*itr);
                    set_pending_payout(result);
                    result.active_votes = get_active_votes(author, permlink);
                    return result;
                }
                return discussion();
            });
        }

        DEFINE_API(database_api,get_active_votes){
            return my->database().with_read_lock([&]() {
                std::vector<vote_state> result;
                const auto &comment = my->database().get_comment(author, permlink);
                const auto &idx = my->database().get_index<comment_vote_index>().indices().get<by_comment_voter>();
                comment_object::id_type cid(comment.id);
                auto itr = idx.lower_bound(cid);
                while (itr != idx.end() && itr->comment == cid) {
                    const auto &vo = my->_db.get(itr->voter);
                    vote_state vstate;
                    vstate.voter = vo.name;
                    vstate.weight = itr->weight;
                    vstate.rshares = itr->rshares;
                    vstate.percent = itr->vote_percent;
                    vstate.time = itr->last_update;

                    if (my->_follow_api) {
                        auto reps = my->_follow_api->get_account_reputations(vo.name, 1);
                        if (reps.size()) {
                            vstate.reputation = reps[0].reputation;
                        }
                    }

                    result.emplace_back(vstate);
                    ++itr;
                }
                return result;
            });
        }

        DEFINE_API(database_api,get_account_votes){
            return my->database().with_read_lock([&]() {
                std::vector<account_vote> result;

                const auto &voter_acnt = my->database().get_account(voter);
                const auto &idx = my->database().get_index<comment_vote_index>().indices().get<by_voter_comment>();

                account_object::id_type aid(voter_acnt.id);
                auto itr = idx.lower_bound(aid);
                auto end = idx.upper_bound(aid);
                while (itr != end) {
                    const auto &vo = my->database().get(itr->comment);
                    account_vote avote;
                    avote.authorperm = vo.author + "/" + to_string(vo.permlink);
                    avote.weight = itr->weight;
                    avote.rshares = itr->rshares;
                    avote.percent = itr->vote_percent;
                    avote.time = itr->last_update;
                    result.emplace_back(avote);
                    ++itr;
                }
                return result;
            });
        }

        boost::multiprecision::uint256_t to256(const fc::uint128 &t) {
            boost::multiprecision::uint256_t result(t.high_bits());
            result <<= 65;
            result += t.low_bits();
            return result;
        }

        void database_api::set_pending_payout(discussion &d) const {
            const auto &cidx = my->database().get_index<tags::tag_index>().indices().get<tags::by_comment>();
            auto itr = cidx.lower_bound(d.id);
            if (itr != cidx.end() && itr->comment == d.id) {
                d.promoted = asset(itr->promoted_balance, SBD_SYMBOL);
            }

            const auto &props = my->database().get_dynamic_global_properties();
            const auto &hist = my->database().get_feed_history();
            asset pot;
            if (my->database().has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                pot = my->database().get_reward_fund(my->database().get_comment(d.author, d.permlink)).reward_balance;
            } else {
                pot = props.total_reward_fund_steem;
            }

            if (!hist.current_median_history.is_null()) {
                pot = pot * hist.current_median_history;
            }

            boost::multiprecision::uint256_t total_r2 = 0;
            if (my->database().has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                total_r2 = to256(my->database().get_reward_fund(my->database().get_comment(d.author, d.permlink)).recent_claims);
            } else {
                total_r2 = to256(props.total_reward_shares2);
            }

            if (props.total_reward_shares2 > 0) {
                uint128_t vshares;
                if (my->database().has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                    vshares = steemit::chain::utilities::calculate_claims(
                            d.net_rshares.value > 0 ? d.net_rshares.value : 0,
                            my->database().get_reward_fund(my->database().get_comment(d.author, d.permlink)));
                } else {
                    vshares = steemit::chain::utilities::calculate_claims(
                            d.net_rshares.value > 0 ? d.net_rshares.value : 0);
                }

                boost::multiprecision::uint256_t r2 = to256(vshares); //to256(abs_net_rshares);
                /*
      r2 *= r2;
      */
                r2 *= pot.amount.value;
                r2 /= total_r2;

                boost::multiprecision::uint256_t tpp = to256(d.children_rshares2);
                tpp *= pot.amount.value;
                tpp /= total_r2;

                d.pending_payout_value = asset(static_cast<uint64_t>(r2), pot.symbol);
                d.total_pending_payout_value = asset(static_cast<uint64_t>(tpp), pot.symbol);

                if (my->_follow_api) {
                    d.author_reputation = my->_follow_api->get_account_reputations(d.author, 1)[0].reputation;
                }
            }

            if (d.parent_author != STEEMIT_ROOT_POST_PARENT) {
                d.cashout_time = my->database().calculate_discussion_payout_time(my->database().get<comment_object>(d.id));
            }

            if (d.body.size() > 1024 * 128) {
                d.body = "body pruned due to size";
            }
            if (d.parent_author.size() > 0 && d.body.size() > 1024 * 16) {
                d.body = "comment pruned due to size";
            }

            set_url(d);
        }

        void database_api::set_url(discussion &d) const {
            const comment_api_obj root(my->database().get<comment_object, by_id>(d.root_comment));
            d.url = "/" + root.category + "/@" + root.author + "/" + root.permlink;
            d.root_title = root.title;
            if (root.id != d.id) {
                d.url += "#@" + d.author + "/" + d.permlink;
            }
        }

        DEFINE_API(database_api,get_content_replies){
            return my->database().with_read_lock([&]() {
                account_name_type acc_name = account_name_type(author);
                const auto &by_permlink_idx = my->database().get_index<comment_index>().indices().get<by_parent>();
                auto itr = by_permlink_idx.find(boost::make_tuple(acc_name, permlink));
                std::vector<discussion> result;
                while (itr != by_permlink_idx.end() && itr->parent_author == author && to_string(itr->parent_permlink) == permlink) {

                    discussion push_discussion(*itr);
                    push_discussion.active_votes = get_active_votes(author, permlink);

                    result.emplace_back(*itr);
                    set_pending_payout(result.back());
                    ++itr;
                }
                return result;
            });
        }

/**
 *  This method can be used to fetch replies to an account.
 *
 *  The first call should be (account_to_retrieve replies, "", limit)
 *  Subsequent calls should be (last_author, last_permlink, limit)
 */
        DEFINE_API(database_api,get_replies_by_last_update){
            return my->database().with_read_lock([&]() {
                std::vector<discussion> result;

#ifndef STEEMIT_BUILD_LOW_MEMORY
                FC_ASSERT(limit <= 100);
                const auto &last_update_idx = my->database().get_index<comment_index>().indices().get<by_last_update>();
                auto itr = last_update_idx.begin();
                const account_name_type *parent_author = &start_parent_author;

                if (start_permlink.size()) {
                    const auto &comment = my->database().get_comment(start_parent_author, start_permlink);
                    itr = last_update_idx.iterator_to(comment);
                    parent_author = &comment.parent_author;
                } else if (start_parent_author.size()) {
                    itr = last_update_idx.lower_bound(start_parent_author);
                }

                result.reserve(limit);

                while (itr != last_update_idx.end() && result.size() < limit &&
                       itr->parent_author == *parent_author) {
                    result.emplace_back(*itr);
                    set_pending_payout(result.back());
                    result.back().active_votes = get_active_votes(itr->author, to_string(itr->permlink));
                    ++itr;
                }

#endif
                return result;
            });
        }

        DEFINE_API(database_api,get_account_history){
            return my->database().with_read_lock([&]() {
                FC_ASSERT(limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l", limit));
                FC_ASSERT(from >= limit, "From must be greater than limit");
                //   idump((account)(from)(limit));
                const auto &idx = my->database().get_index<account_history_index>().indices().get<by_account>();
                auto itr = idx.lower_bound(boost::make_tuple(account, from));
                //   if( itr != idx.end() ) idump((*itr));
                auto end = idx.upper_bound(boost::make_tuple(account, std::max(int64_t(0), int64_t(itr->sequence) - limit)));
                //   if( end != idx.end() ) idump((*end));

                std::map<uint32_t, applied_operation> result;
                while (itr != end) {
                    result[itr->sequence] = my->_db.get(itr->op);
                    ++itr;
                }
                return result;
            });
        }

        DEFINE_API(database_api,get_payout_extension_cost){
            return my->database().get_payout_extension_cost(my->database().get_comment(author, permlink), time);
        }

        DEFINE_API(database_api,get_payout_extension_time){
            return my->database().get_payout_extension_time(my->database().get_comment(author, permlink), cost);
        }

            DEFINE_API(database_api,get_tags_used_by_author){
            return my->database().with_read_lock([&]() {
                const auto *acnt = my->database().find_account(author);
                FC_ASSERT(acnt != nullptr);
                const auto &tidx = my->database().get_index<tags::author_tag_stats_index>().indices().get<tags::by_author_posts_tag>();
                auto itr = tidx.lower_bound(boost::make_tuple(acnt->id, 0));
                std::vector<pair<std::string, uint32_t>> result;
                while (itr != tidx.end() && itr->author == acnt->id && result.size() < 1000) {
                    if (!fc::is_utf8(itr->tag)) {
                        result.emplace_back(std::make_pair(fc::prune_invalid_utf8(itr->tag), itr->total_posts));
                    } else {
                        result.emplace_back(std::make_pair(itr->tag, itr->total_posts));
                    }
                    ++itr;
                }
                return result;
            });
        }

            DEFINE_API(database_api,get_trending_tags){
            return my->database().with_read_lock([&]() {
                limit = std::min(limit, uint32_t(1000));
                std::vector<tag_api_obj> result;
                result.reserve(limit);

                const auto &nidx = my->database().get_index<tags::tag_stats_index>().indices().get<tags::by_tag>();

                const auto &ridx = my->database().get_index<tags::tag_stats_index>().indices().get<tags::by_trending>();
                auto itr = ridx.begin();
                if (after != "" && nidx.size()) {
                    auto nitr = nidx.lower_bound(after);
                    if (nitr == nidx.end()) {
                        itr = ridx.end();
                    } else {
                        itr = ridx.iterator_to(*nitr);
                    }
                }

                while (itr != ridx.end() && result.size() < limit) {
                    tag_api_object push_object = tag_api_object(*itr);

                    if (!fc::is_utf8(push_object.name)) {
                        push_object.name = fc::prune_invalid_utf8(push_object.name);
                    }

                    result.emplace_back(push_object);
                    ++itr;
                }
                return result;
            });
        }

        discussion database_api::get_discussion(comment_object::id_type id, uint32_t truncate_body) const {
            discussion d = my->database().get(id);
            set_url(d);
            set_pending_payout(d);
            d.active_votes = get_active_votes(d.author, d.permlink);
            d.body_length = static_cast<uint32_t>(d.body.size());
            if (truncate_body) {
                d.body = d.body.substr(0, truncate_body);

                if (!fc::is_utf8(d.title)) {
                    d.title = fc::prune_invalid_utf8(d.title);
                }

                if (!fc::is_utf8(d.body)) {
                    d.body = fc::prune_invalid_utf8(d.body);
                }

                if (!fc::is_utf8(d.category)) {
                    d.category = fc::prune_invalid_utf8(d.category);
                }

                if (!fc::is_utf8(d.json_metadata)) {
                    d.json_metadata = fc::prune_invalid_utf8(d.json_metadata);
                }

            }
            return d;
        }

        template<
                typename Object,
                typename DatabaseIndex,
                typename DiscussionIndex,
                typename CommentIndex,
                typename Index,
                typename StartItr
        >
        std::multimap<Object, discussion, DiscussionIndex> database_api::get_discussions(
                const discussion_query &query,
                const std::string &tag,
                comment_object::id_type parent,
                const Index &tidx,
                StartItr tidx_itr,
                const std::function<bool(const comment_api_obj &c)> &filter,
                const std::function<bool(const comment_api_obj &c)> &exit,
                const std::function<bool(const Object &)> &tag_exit,
                bool ignore_parent) const {
//   idump((query));

            const auto &cidx = my->database().get_index<DatabaseIndex>().indices().template get<CommentIndex>();
            comment_object::id_type start;


            if (query.start_author && query.start_permlink) {
                start = my->database().get_comment(*query.start_author, *query.start_permlink).id;
                auto itr = cidx.find(start);
                while (itr != cidx.end() && itr->comment == start) {
                    if (itr->name == tag) {
                        tidx_itr = tidx.iterator_to(*itr);
                        break;
                    }
                    ++itr;
                }
            }
            std::multimap<Object, discussion, DiscussionIndex> result;
            uint32_t count = query.limit;
            uint64_t filter_count = 0;
            uint64_t exc_count = 0;
            while (count > 0 && tidx_itr != tidx.end()) {
                if (tidx_itr->name != tag ||
                    (!ignore_parent && tidx_itr->parent != parent)) {
                    break;
                }

                try {
                    discussion insert_discussion = get_discussion(tidx_itr->comment, query.truncate_body);
                    insert_discussion.promoted = asset(tidx_itr->promoted_balance, SBD_SYMBOL);

                    if (filter(insert_discussion)) {
                        ++filter_count;
                    } else if (exit(insert_discussion) || tag_exit(*tidx_itr)) {
                        break;
                    } else {
                        result.insert({*tidx_itr, insert_discussion});
                        --count;
                    }
                } catch (const fc::exception &e) {
                    ++exc_count;
                    edump((e.to_detail_string()));
                }

                ++tidx_itr;
            }
            return result;
        }

        comment_object::id_type database_api::get_parent(const discussion_query &query) const {
            return my->database().with_read_lock([&]() {
                comment_object::id_type parent;
                if (query.parent_author && query.parent_permlink) {
                    parent = my->_db.get_comment(*query.parent_author, *query.parent_permlink).id;
                }
                return parent;
            });
        }

        template<typename FirstCompare, typename SecondCompare>
        std::vector<discussion> merge(std::multimap<tags::tag_object, discussion, FirstCompare> &result1,
                std::multimap<languages::language_object, discussion, SecondCompare> &result2) {
            //TODO:std::set_intersection(
            std::vector<discussion> discussions;
            if (!result2.empty()) {
                std::multimap<comment_object::id_type, discussion> tmp;

                for (auto &&i:result1) {
                    tmp.emplace(i.second.id, std::move(i.second));
                }

                for (auto &&i:result2) {
                    if (tmp.count(i.second.id)) {
                        discussions.push_back(std::move(i.second));
                    }
                }

                return discussions;
            }

            discussions.reserve(result1.size());
            for (auto &&iterator : result1) {
                discussions.push_back(std::move(iterator.second));
            }

            return discussions;
        }

        DEFINE_API(database_api,get_discussions_by_trending){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);

                std::multimap<tags::tag_object,
                        discussion,
                        tags::by_parent_trending
                > map_result = select<tags::tag_object, tags::tag_index, tags::by_parent_trending, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1,
                                  [&](const comment_api_obj &c) -> bool {
                                      return c.net_rshares <= 0;
                                  }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        },
                        parent,
                        std::numeric_limits<double>::max()
                );

                std::multimap<languages::language_object,
                        discussion,
                        languages::by_parent_trending
                > map_result_ = select<languages::language_object, languages::language_index, languages::by_parent_trending, languages::by_comment>(
                        query.select_languages,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1,
                                  [&](const comment_api_obj &c) -> bool {
                                      return c.net_rshares <= 0;
                                  }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        parent,
                        std::numeric_limits<double>::max()
                );


                std::vector<discussion> return_result = merge(map_result, map_result_);

                return return_result;
            });
        }


            DEFINE_API(database_api,get_post_discussions_by_payout){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = comment_object::id_type();

                std::multimap<tags::tag_object, discussion, tags::by_parent_promoted> map_result = select<tags::tag_object, tags::tag_index, tags::by_parent_promoted, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.net_rshares <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &t) {
                            return false;
                        },
                        true
                );

                std::multimap<languages::language_object, discussion, languages::by_parent_promoted> map_result_language = select<languages::language_object, languages::language_index, languages::by_parent_promoted, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.net_rshares <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &t) {
                            return false;
                        },
                        true
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);

                return return_result;
            });
        }

        DEFINE_API(database_api,get_comment_discussions_by_payout){
            return my->database().with_read_lock([&]() {
                query.validate();

                auto parent = comment_object::id_type(1);

                std::multimap<tags::tag_object, discussion, tags::by_reward_fund_net_rshares> map_result = select<tags::tag_object, tags::tag_index, tags::by_reward_fund_net_rshares, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.net_rshares <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &t) {
                            return false;
                        },
                        false
                );

                std::multimap<languages::language_object, discussion, languages::by_reward_fund_net_rshares> map_result_language = select<languages::language_object, languages::language_index, languages::by_reward_fund_net_rshares, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.net_rshares <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &t) {
                            return false;
                        },
                        false
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);


                return return_result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_promoted){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);

                std::multimap<tags::tag_object, discussion, tags::by_parent_promoted> map_result = select<tags::tag_object, tags::tag_index, tags::by_parent_promoted, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.children_rshares2 <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        },
                        parent,
                        share_type(STEEMIT_MAX_SHARE_SUPPLY)
                );


                std::multimap<languages::language_object, discussion, languages::by_parent_promoted> map_result_language = select<languages::language_object, languages::language_index, languages::by_parent_promoted, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.children_rshares2 <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        parent,
                        share_type(STEEMIT_MAX_SHARE_SUPPLY)
                );


                std::vector<discussion> return_result = merge(map_result, map_result_language);


                return return_result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_created){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);

                std::multimap<tags::tag_object, discussion, tags::by_parent_created> map_result = select<tags::tag_object, tags::tag_index, tags::by_parent_created, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return false;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        },
                        parent,
                        fc::time_point_sec::maximum()
                );

                std::multimap<languages::language_object, discussion, languages::by_parent_created> map_result_language = select<languages::language_object, languages::language_index, languages::by_parent_created, languages::by_comment>(query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return false;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        parent,
                        fc::time_point_sec::maximum()
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);

                return return_result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_active){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);


                std::multimap<tags::tag_object, discussion, tags::by_parent_active> map_result = select<tags::tag_object, tags::tag_index, tags::by_parent_active, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return false;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        },
                        parent,
                        fc::time_point_sec::maximum()
                );

                std::multimap<languages::language_object, discussion, languages::by_parent_active> map_result_language = select<languages::language_object, languages::language_index, languages::by_parent_active, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return false;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        parent,
                        fc::time_point_sec::maximum()
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);

                return return_result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_cashout){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);


                std::multimap<tags::tag_object, discussion, tags::by_cashout> map_result = select<tags::tag_object, tags::tag_index, tags::by_cashout, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.children_rshares2 <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        },
                        fc::time_point::now() - fc::minutes(60)
                );

                std::multimap<languages::language_object, discussion, languages::by_cashout> map_result_language = select<languages::language_object, languages::language_index, languages::by_cashout, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.children_rshares2 <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        fc::time_point::now() - fc::minutes(60)
                );


                std::vector<discussion> return_result = merge(map_result, map_result_language);
                return return_result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_payout){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);

                std::multimap<tags::tag_object, discussion, tags::by_net_rshares> map_result = select<tags::tag_object, tags::tag_index, tags::by_net_rshares, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.children_rshares2 <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        }
                );

                std::multimap<languages::language_object, discussion, languages::by_net_rshares> map_result_language = select<languages::language_object, languages::language_index, languages::by_net_rshares, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.children_rshares2 <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        }
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);

                return return_result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_votes){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);

                std::multimap<tags::tag_object, discussion, tags::by_parent_net_votes> map_result;

                select<tags::tag_object, tags::tag_index, tags::by_parent_net_votes, tags::by_comment>(query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return false;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        },
                        parent,
                        std::numeric_limits<int32_t>::max()
                );

                std::multimap<languages::language_object, discussion, languages::by_parent_net_votes> map_result_language = select<languages::language_object, languages::language_index, languages::by_parent_net_votes, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return false;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        parent,
                        std::numeric_limits<int32_t>::max()
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);

                return return_result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_children){
            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);

                std::multimap<tags::tag_object, discussion, tags::by_parent_children> map_result =

                        select<tags::tag_object, tags::tag_index, tags::by_parent_children, tags::by_comment>(
                                query.select_tags,
                                query,
                                parent,
                                std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                                    return false;
                                }),
                                [&](const comment_api_obj &c) -> bool {
                                    return false;
                                },
                                [&](const tags::tag_object &) -> bool {
                                    return false;
                                },
                                parent,
                                std::numeric_limits<int32_t>::max()
                        );

                std::multimap<languages::language_object, discussion, languages::by_parent_children> map_result_language = select<languages::language_object, languages::language_index, languages::by_parent_children, languages::by_comment>(query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return false;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        parent,
                        std::numeric_limits<int32_t>::max()
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);

                return return_result;
            });
        }

            DEFINE_API(database_api,get_discussions_by_hot){

            return my->database().with_read_lock([&]() {
                query.validate();
                auto parent = get_parent(query);

                std::multimap<tags::tag_object, discussion, tags::by_parent_hot> map_result = select<tags::tag_object, tags::tag_index, tags::by_parent_hot, tags::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(tags::tags_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.net_rshares <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const tags::tag_object &) -> bool {
                            return false;
                        },
                        parent,
                        std::numeric_limits<double>::max()
                );

                std::multimap<languages::language_object, discussion, languages::by_parent_hot> map_result_language = select<languages::language_object, languages::language_index, languages::by_parent_hot, languages::by_comment>(
                        query.select_tags,
                        query,
                        parent,
                        std::bind(languages::languages_plugin::filter, query, std::placeholders::_1, [&](const comment_api_obj &c) -> bool {
                            return c.net_rshares <= 0;
                        }),
                        [&](const comment_api_obj &c) -> bool {
                            return false;
                        },
                        [&](const languages::language_object &) -> bool {
                            return false;
                        },
                        parent,
                        std::numeric_limits<double>::max()
                );

                std::vector<discussion> return_result = merge(map_result, map_result_language);

                return return_result;
            });
        }

        std::vector<discussion> merge(std::vector<discussion> &result1, std::vector<discussion> &result2) {
            //TODO:std::set_intersection(
            if (!result2.empty()) {
                std::vector<discussion> discussions;
                std::multimap<comment_object::id_type, discussion> tmp;

                for (auto &&i:result1) {
                    tmp.emplace(i.id, std::move(i));
                }


                for (auto &&i:result2) {
                    if (tmp.count(i.id)) {
                        discussions.push_back(std::move(i));
                    }
                }

                return discussions;
            }

            std::vector<discussion> discussions(result1.begin(), result1.end());
            return discussions;
        }

        template<
                typename DatabaseIndex,
                typename DiscussionIndex
        >
        std::vector<discussion> database_api::feed(
                const std::set<string> &select_set,
                const discussion_query &query,
                const std::string &start_author,
                const std::string &start_permlink) const
        {
            std::vector<discussion> result;

            for (const auto &iterator : query.select_authors) {
                const auto &account = my->_db.get_account(iterator);

                const auto &tag_idx = my->_db.get_index<DatabaseIndex>().indices().template get<DiscussionIndex>();

                const auto &c_idx = my->_db.get_index<follow::feed_index>().indices().get<follow::by_comment>();
                const auto &f_idx = my->_db.get_index<follow::feed_index>().indices().get<follow::by_feed>();
                auto feed_itr = f_idx.lower_bound(account.name);

                if (start_author.size() || start_permlink.size()) {
                    auto start_c = c_idx.find(boost::make_tuple(my->_db.get_comment(start_author, start_permlink).id, account.name));
                    FC_ASSERT(start_c !=
                              c_idx.end(), "Comment is not in account's feed");
                    feed_itr = f_idx.iterator_to(*start_c);
                }

                while (result.size() < query.limit && feed_itr != f_idx.end()) {
                    if (feed_itr->account != account.name) {
                        break;
                    }
                    try {
                        if (!select_set.empty()) {
                            auto tag_itr = tag_idx.lower_bound(feed_itr->comment);

                            bool found = false;
                            while (tag_itr != tag_idx.end() &&
                                   tag_itr->comment == feed_itr->comment) {
                                if (select_set.find(tag_itr->name) !=
                                    select_set.end()) {
                                    found = true;
                                    break;
                                }
                                ++tag_itr;
                            }
                            if (!found) {
                                ++feed_itr;
                                continue;
                            }
                        }

                        result.push_back(get_discussion(feed_itr->comment));
                        if (feed_itr->first_reblogged_by !=
                            account_name_type()) {
                            result.back().reblogged_by = std::vector<account_name_type>(feed_itr->reblogged_by.begin(), feed_itr->reblogged_by.end());
                            result.back().first_reblogged_by = feed_itr->first_reblogged_by;
                            result.back().first_reblogged_on = feed_itr->first_reblogged_on;
                        }
                    }
                    catch (const fc::exception &e) {
                        edump((e.to_detail_string()));
                    }

                    ++feed_itr;
                }
            }

            return result;
        }

        DEFINE_API(database_api,get_discussions_by_feed){
            return my->database().with_read_lock([&]() {
                query.validate();
                FC_ASSERT(my->_follow_api, "Node is not running the follow_api plugin");
                FC_ASSERT(query.select_authors.size(), "No such author to select feed from");

                std::string start_author = query.start_author ? *(query.start_author) : "";
                std::string start_permlink = query.start_permlink ? *(query.start_permlink) : "";

                std::vector<discussion> tags_ = feed<
                        tags::tag_index,
                        tags::by_comment
                >(query.select_tags, query, start_author, start_permlink);

                std::vector<discussion> languages_ = feed<
                        languages::language_index,
                        languages::by_comment
                >(query.select_languages, query, start_author, start_permlink);

                std::vector<discussion> result = merge(tags_, languages_);

                return result;
            });
        }

        template<typename DatabaseIndex,
                typename DiscussionIndex
        >
        std::vector<discussion> database_api::blog(
                const std::set<string> &select_set,
                const discussion_query &query,
                const std::string &start_author,
                const std::string &start_permlink
        ) const {
            std::vector<discussion> result;
            for (const auto &iterator : query.select_authors) {

                const auto &account = my->_db.get_account(iterator);

                const auto &tag_idx = my->_db.get_index<DatabaseIndex>().indices().template get<DiscussionIndex>();

                const auto &c_idx = my->_db.get_index<follow::blog_index>().indices().get<follow::by_comment>();
                const auto &b_idx = my->_db.get_index<follow::blog_index>().indices().get<follow::by_blog>();
                auto blog_itr = b_idx.lower_bound(account.name);

                if (start_author.size() || start_permlink.size()) {
                    auto start_c = c_idx.find(boost::make_tuple(my->_db.get_comment(start_author, start_permlink).id, account.name));
                    FC_ASSERT(start_c !=
                              c_idx.end(), "Comment is not in account's blog");
                    blog_itr = b_idx.iterator_to(*start_c);
                }

                while (result.size() < query.limit && blog_itr != b_idx.end()) {
                    if (blog_itr->account != account.name) {
                        break;
                    }
                    try {
                        if (select_set.size()) {
                            auto tag_itr = tag_idx.lower_bound(blog_itr->comment);

                            bool found = false;
                            while (tag_itr != tag_idx.end() &&
                                   tag_itr->comment == blog_itr->comment) {
                                if (select_set.find(tag_itr->name) !=
                                    select_set.end()) {
                                    found = true;
                                    break;
                                }
                                ++tag_itr;
                            }
                            if (!found) {
                                ++blog_itr;
                                continue;
                            }
                        }

                        result.push_back(get_discussion(blog_itr->comment, query.truncate_body));
                        if (blog_itr->reblogged_on > time_point_sec()) {
                            result.back().first_reblogged_on = blog_itr->reblogged_on;
                        }
                    }
                    catch (const fc::exception &e) {
                        edump((e.to_detail_string()));
                    }

                    ++blog_itr;
                }
            }
            return result;
        }

        DEFINE_API(database_api,get_discussions_by_blog){
            return my->databse().with_read_lock([&]() {
                query.validate();
                FC_ASSERT(my->_follow_api, "Node is not running the follow_api plugin");
                FC_ASSERT(query.select_authors.size(), "No such author to select feed from");

                auto start_author = query.start_author ? *(query.start_author)
                                                       : "";
                auto start_permlink = query.start_permlink
                                      ? *(query.start_permlink) : "";

                std::vector<discussion> tags_ = blog<tags::tag_index, tags::by_comment>(query.select_tags, query, start_author, start_permlink);

                std::vector<discussion> languages_ = blog<languages::language_index, languages::by_comment>(query.select_languages, query, start_author, start_permlink);

                std::vector<discussion> result = merge(tags_, languages_);

                return result;
            });
        }

        DEFINE_API(database_api,get_discussions_by_comments){
            return my->database().with_read_lock([&]() {
                std::vector<discussion> result;
#ifndef STEEMIT_BUILD_LOW_MEMORY
                query.validate();
                FC_ASSERT(query.start_author, "Must get comments for a specific author");
                auto start_author = *(query.start_author);
                auto start_permlink = query.start_permlink
                                      ? *(query.start_permlink) : "";

                const auto &c_idx = my->_db.get_index<comment_index>().indices().get<by_permlink>();
                const auto &t_idx = my->_db.get_index<comment_index>().indices().get<by_author_last_update>();
                auto comment_itr = t_idx.lower_bound(start_author);

                if (start_permlink.size()) {
                    auto start_c = c_idx.find(boost::make_tuple(start_author, start_permlink));
                    FC_ASSERT(start_c !=
                              c_idx.end(), "Comment is not in account's comments");
                    comment_itr = t_idx.iterator_to(*start_c);
                }

                result.reserve(query.limit);

                while (result.size() < query.limit &&
                       comment_itr != t_idx.end()) {
                    if (comment_itr->author != start_author) {
                        break;
                    }
                    if (comment_itr->parent_author.size() > 0) {
                        try {
                            if (query.select_authors.size() &&
                                query.select_authors.find(comment_itr->author) ==
                                query.select_authors.end()) {
                                ++comment_itr;
                                continue;
                            }

                            result.emplace_back(get_discussion(comment_itr->id));
                        }
                        catch (const fc::exception &e) {
                            edump((e.to_detail_string()));
                        }
                    }

                    ++comment_itr;
                }
#endif
                return result;
            });
        }

        DEFINE_API(database_api,get_trending_categories){
            return my->database().with_read_lock([&]() {
                limit = std::min(limit, uint32_t(100));
                std::vector<category_api_obj> result;
                result.reserve(limit);

                const auto &nidx = my->_db.get_index<chain::category_index>().indices().get<by_name>();

                const auto &ridx = my->_db.get_index<chain::category_index>().indices().get<by_rshares>();
                auto itr = ridx.begin();
                if (after != "" && nidx.size()) {
                    auto nitr = nidx.lower_bound(after);
                    if (nitr == nidx.end()) {
                        itr = ridx.end();
                    } else {
                        itr = ridx.iterator_to(*nitr);
                    }
                }

                while (itr != ridx.end() && result.size() < limit) {
                    result.emplace_back(category_api_obj(*itr));
                    ++itr;
                }
                return result;
            });
        }

        DEFINE_API(database_api,get_best_categories){
            return my->database().with_read_lock([&]() {
                limit = std::min(limit, uint32_t(100));
                std::vector<category_api_obj> result;
                result.reserve(limit);
                return result;
            });
        }

        DEFINE_API(database_api,get_active_categories){
            return my->database().with_read_lock([&]() {
                limit = std::min(limit, uint32_t(100));
                std::vector<category_api_obj> result;
                result.reserve(limit);
                return result;
            });
        }

        DEFINE_API(database_api,get_recent_categories){
            return my->database().with_read_lock([&]() {
                limit = std::min(limit, uint32_t(100));
                std::vector<category_api_obj> result;
                result.reserve(limit);
                return result;
            });
        }


/**
 *  This call assumes root already stored as part of state, it will
 *  modify root.replies to contain links to the reply posts and then
 *  add the reply discussions to the state. This method also fetches
 *  any accounts referenced by authors.
 *
 */
        void database_api::recursively_fetch_content(state &_state, discussion &root, std::set<std::string> &referenced_accounts) const {
            return my->database().with_read_lock([&]() {
                try {
                    if (root.author.size()) {
                        referenced_accounts.insert(root.author);
                    }

                    auto replies = get_content_replies(root.author, root.permlink);
                    for (auto &r : replies) {
                        try {
                            recursively_fetch_content(_state, r, referenced_accounts);
                            root.replies.push_back(r.author + "/" + r.permlink);
                            _state.content[r.author + "/" +
                                           r.permlink] = std::move(r);
                            if (r.author.size()) {
                                referenced_accounts.insert(r.author);
                            }
                        }
                        catch (const fc::exception &e) {
                            edump((e.to_detail_string()));
                        }
                    }
                }
                FC_CAPTURE_AND_RETHROW((root.author)(root.permlink))
            });
        }

        DEFINE_API(database_api,get_miner_queue){
            return my->database().with_read_lock([&]() {
                std::vector<account_name_type> result;
                const auto &pow_idx = my->database().get_index<witness_index>().indices().get<by_pow>();

                auto itr = pow_idx.upper_bound(0);
                while (itr != pow_idx.end()) {
                    if (itr->pow_worker) {
                        result.push_back(itr->owner);
                    }
                    ++itr;
                }
                return result;
            });
        }

            DEFINE_API(database_api,get_active_witnesses){
                return my->database().with_read_lock([&]() {
                    const auto &wso = my->database().get_witness_schedule_object();
                    size_t n = wso.current_shuffled_witnesses.size();
                    vector<account_name_type> result;
                    result.reserve(n);
                    for (size_t i = 0; i < n; i++) {
                        result.push_back(wso.current_shuffled_witnesses[i]);
                    }
                    return result;
                });
            }

            DEFINE_API(database_api,get_discussions_by_author_before_date){
            return my->database().with_read_lock([&]() {
                try {
                    std::vector<discussion> result;
#ifndef STEEMIT_BUILD_LOW_MEMORY
                    FC_ASSERT(limit <= 100);
                    result.reserve(limit);
                    uint32_t count = 0;
                    const auto &didx = my->database().get_index<comment_index>().indices().get<by_author_last_update>();

                    if (before_date == time_point_sec()) {
                        before_date = time_point_sec::maximum();
                    }

                    auto itr = didx.lower_bound(boost::make_tuple(author, time_point_sec::maximum()));
                    if (start_permlink.size()) {
                        const auto &comment = my->database().get_comment(author, start_permlink);
                        if (comment.created < before_date) {
                            itr = didx.iterator_to(comment);
                        }
                    }


                    while (itr != didx.end() && itr->author == author && count < limit) {
                        if (itr->parent_author.size() == 0) {
                            result.push_back(*itr);
                            set_pending_payout(result.back());
                            result.back().active_votes = get_active_votes(itr->author, to_string(itr->permlink));
                            ++count;
                        }
                        ++itr;
                    }

#endif
                    return result;
                }
                FC_CAPTURE_AND_RETHROW((author)(start_permlink)(before_date)(limit))
            });
        }

        DEFINE_API(database_api,get_savings_withdraw_from){
            return my->database().with_read_lock([&]() {
                std::vector<savings_withdraw_api_object> result;

                const auto &from_rid_idx = my->database().get_index<savings_withdraw_index>().indices().get<by_from_rid>();
                auto itr = from_rid_idx.lower_bound(account);
                while (itr != from_rid_idx.end() && itr->from == account) {
                    result.push_back(savings_withdraw_api_object(*itr));
                    ++itr;
                }
                return result;
            });
        }

        DEFINE_API(database_api::get_savings_withdraw_to){
            return my->database().with_read_lock([&]() {
                std::vector<savings_withdraw_api_object> result;

                const auto &to_complete_idx = my->database().get_index<savings_withdraw_index>().indices().get<by_to_complete>();
                auto itr = to_complete_idx.lower_bound(account);
                while (itr != to_complete_idx.end() && itr->to == account) {
                    result.push_back(savings_withdraw_api_object(*itr));
                    ++itr;
                }
                return result;
            });
        }

        DEFINE_API(database_api,get_vesting_delegations){
            FC_ASSERT(limit <= 1000);

            return my->database().with_read_lock([&]() {
                vector<vesting_delegation_api_object> result;
                result.reserve(limit);

                const auto &delegation_idx = my->database().get_index<vesting_delegation_index, by_delegation>();
                auto itr = delegation_idx.lower_bound(boost::make_tuple(account, from));
                while (result.size() < limit && itr != delegation_idx.end() &&
                       itr->delegator == account) {
                    result.push_back(*itr);
                    ++itr;
                }

                return result;
            });
        }

        DEFINE_API(database_api,get_expiring_vesting_delegations){
            FC_ASSERT(limit <= 1000);

            return my->database().with_read_lock([&]() {
                vector<vesting_delegation_expiration_api_object> result;
                result.reserve(limit);

                const auto &exp_idx = my->database().get_index<vesting_delegation_expiration_index, by_account_expiration>();
                auto itr = exp_idx.lower_bound(boost::make_tuple(account, from));
                while (result.size() < limit && itr != exp_idx.end() &&
                       itr->delegator == account) {
                    result.push_back(*itr);
                    ++itr;
                }

                return result;
            });
        }

        DEFINE_API(database_api,get_state){
            return my->database().with_read_lock([&]() {
                state _state;
                _state.props = get_dynamic_global_properties();
                _state.current_route = path;
                _state.feed_price = get_current_median_history_price();

                try {
                    if (path.size() && path[0] == '/') {
                        path = path.substr(1);
                    } /// remove '/' from front

                    if (!path.size()) {
                        path = "trending";
                    }

                    /// FETCH CATEGORY STATE
                    auto trending_tags = get_trending_tags(std::string(), 50);
                    for (const auto &t : trending_tags) {
                        _state.tag_idx.trending.push_back(std::string(t.name));
                    }
                    /// END FETCH CATEGORY STATE

                    std::set<std::string> accounts;

                    std::vector<std::string> part;
                    part.reserve(4);
                    boost::split(part, path, boost::is_any_of("/"));
                    part.resize(std::max(part.size(), size_t(4))); // at least 4

                    auto tag = fc::to_lower(part[1]);

                    if (part[0].size() && part[0][0] == '@') {
                        auto acnt = part[0].substr(1);
                        _state.accounts[acnt] = extended_account(my->database().get_account(acnt), my->database());
                        _state.accounts[acnt].tags_usage = get_tags_used_by_author(acnt);
                        if (my->_follow_api) {
                            _state.accounts[acnt].guest_bloggers = my->_follow_api->get_blog_authors(acnt);
                            _state.accounts[acnt].reputation = my->_follow_api->get_account_reputations(acnt, 1)[0].reputation;
                        }
                        auto &eacnt = _state.accounts[acnt];
                        if (part[1] == "transfers") {
                            auto history = get_account_history(acnt, uint64_t(-1), 1000);
                            for (auto &item : history) {
                                switch (item.second.op.which()) {
                                    case operation::tag<transfer_to_vesting_operation>::value:
                                    case operation::tag<withdraw_vesting_operation>::value:
                                    case operation::tag<interest_operation>::value:
                                    case operation::tag<transfer_operation>::value:
                                    case operation::tag<liquidity_reward_operation>::value:
                                    case operation::tag<author_reward_operation>::value:
                                    case operation::tag<curation_reward_operation>::value:
                                    case operation::tag<comment_benefactor_reward_operation>::value:
                                    case operation::tag<transfer_to_savings_operation>::value:
                                    case operation::tag<transfer_from_savings_operation>::value:
                                    case operation::tag<cancel_transfer_from_savings_operation>::value:
                                    case operation::tag<escrow_transfer_operation>::value:
                                    case operation::tag<escrow_approve_operation>::value:
                                    case operation::tag<escrow_dispute_operation>::value:
                                    case operation::tag<escrow_release_operation>::value:
                                    case operation::tag<fill_convert_request_operation>::value:
                                    case operation::tag<fill_order_operation>::value:
                                        eacnt.transfer_history[item.first] = item.second;
                                        break;
                                    case operation::tag<comment_operation>::value:
                                        //   eacnt.post_history[item.first] =  item.second;
                                        break;
                                    case operation::tag<limit_order_create_operation>::value:
                                    case operation::tag<limit_order_cancel_operation>::value:
                                        //   eacnt.market_history[item.first] =  item.second;
                                        break;
                                    case operation::tag<vote_operation>::value:
                                    case operation::tag<account_witness_vote_operation>::value:
                                    case operation::tag<account_witness_proxy_operation>::value:
                                        //   eacnt.vote_history[item.first] =  item.second;
                                        break;
                                    case operation::tag<account_create_operation>::value:
                                    case operation::tag<account_update_operation>::value:
                                    case operation::tag<witness_update_operation>::value:
                                    case operation::tag<pow_operation>::value:
                                    case operation::tag<custom_operation>::value:
                                    default:
                                        eacnt.other_history[item.first] = item.second;
                                }
                            }
                        } else if (part[1] == "recent-replies") {
                            auto replies = get_replies_by_last_update(acnt, "", 50);
                            eacnt.recent_replies = std::vector<std::string>();
                            for (const auto &reply : replies) {
                                auto reply_ref = reply.author + "/" + reply.permlink;
                                _state.content[reply_ref] = reply;
                                if (my->_follow_api) {
                                    _state.accounts[reply_ref].reputation = my->_follow_api->get_account_reputations(reply.author, 1)[0].reputation;
                                }
                                eacnt.recent_replies->push_back(reply_ref);
                            }
                        } else if (part[1] == "posts" || part[1] == "comments") {
#ifndef STEEMIT_BUILD_LOW_MEMORY
                            int count = 0;
                            const auto &pidx = my->database().get_index<comment_index>().indices().get<by_author_last_update>();
                            auto itr = pidx.lower_bound(acnt);
                            eacnt.comments = std::vector<std::string>();

                            while (itr != pidx.end() && itr->author == acnt &&
                                   count < 20) {
                                if (itr->parent_author.size()) {
                                    const auto link = acnt + "/" + to_string(itr->permlink);
                                    eacnt.comments->push_back(link);
                                    _state.content[link] = *itr;
                                    set_pending_payout(_state.content[link]);
                                    ++count;
                                }

                                ++itr;
                            }
#endif
                        } else if (part[1].size() == 0 || part[1] == "blog") {
                            if (my->_follow_api) {
                                auto blog = my->_follow_api->get_blog_entries(eacnt.name, 0, 20);
                                eacnt.blog = std::vector<std::string>();

                                for (auto b: blog) {
                                    const auto link = b.author + "/" + b.permlink;
                                    eacnt.blog->push_back(link);
                                    _state.content[link] = my->database().get_comment(b.author, b.permlink);
                                    set_pending_payout(_state.content[link]);

                                    if (b.reblog_on > time_point_sec()) {
                                        _state.content[link].first_reblogged_on = b.reblog_on;
                                    }
                                }
                            }
                        } else if (part[1].size() == 0 || part[1] == "feed") {
                            if (my->_follow_api) {
                                auto feed = my->_follow_api->get_feed_entries(eacnt.name, 0, 20);
                                eacnt.feed = std::vector<std::string>();

                                for (auto f: feed) {
                                    const auto link = f.author + "/" + f.permlink;
                                    eacnt.feed->push_back(link);
                                    _state.content[link] = my->database().get_comment(f.author, f.permlink);
                                    set_pending_payout(_state.content[link]);
                                    if (f.reblog_by.size()) {
                                        if (f.reblog_by.size()) {
                                            _state.content[link].first_reblogged_by = f.reblog_by[0];
                                        }
                                        _state.content[link].reblogged_by = f.reblog_by;
                                        _state.content[link].first_reblogged_on = f.reblog_on;
                                    }
                                }
                            }
                        }
                    }
                        /// pull a complete discussion
                    else if (part[1].size() && part[1][0] == '@') {

                        auto account = part[1].substr(1);
                        auto category = part[0];
                        auto slug = part[2];

                        auto key = account + "/" + slug;
                        auto dis = get_content(account, slug);

                        recursively_fetch_content(_state, dis, accounts);
                        _state.content[key] = std::move(dis);
                    } else if (part[0] == "witnesses" ||
                               part[0] == "~witnesses") {
                        auto wits = get_witnesses_by_vote("", 50);
                        for (const auto &w : wits) {
                            _state.witnesses[w.owner] = w;
                        }
                        _state.pow_queue = get_miner_queue();
                    } else if (part[0] == "payout_comments") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;
                        auto trending_disc = get_comment_discussions_by_payout(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.payout_comments.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "payout") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;
                        auto trending_disc = get_post_discussions_by_payout(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.trending.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "promoted") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_promoted(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.promoted.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "responses") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_children(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.responses.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (!part[0].size() || part[0] == "hot") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_hot(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.hot.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (!part[0].size() || part[0] == "promoted") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_promoted(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.promoted.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "votes") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_votes(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.votes.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "cashout") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_cashout(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.cashout.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "active") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_active(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.active.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "created") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_created(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.created.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "recent") {
                        discussion_query q;
                        q.select_tags.insert(tag);
                        q.limit = 20;
                        q.truncate_body = 1024;

                        auto trending_disc = get_discussions_by_created(q);

                        auto &didx = _state.discussion_idx[tag];
                        for (const auto &d : trending_disc) {
                            auto key = d.author + "/" + d.permlink;
                            didx.created.push_back(key);
                            if (d.author.size()) {
                                accounts.insert(d.author);
                            }
                            _state.content[key] = std::move(d);
                        }
                    } else if (part[0] == "tags") {
                        _state.tag_idx.trending.clear();
                        auto trending_tags = get_trending_tags(std::string(), 250);
                        for (const auto &t : trending_tags) {
                            std::string name = t.name;
                            _state.tag_idx.trending.push_back(name);
                            _state.tags[name] = t;
                        }
                    } else {
                        elog("What... no matches");
                    }

                    for (const auto &a : accounts) {
                        _state.accounts.erase("");
                        _state.accounts[a] = extended_account(my->database().get_account(a), my->database());
                        if (my->_follow_api) {
                            _state.accounts[a].reputation = my->_follow_api->get_account_reputations(a, 1)[0].reputation;
                        }
                    }
                    for (auto &d : _state.content) {
                        d.second.active_votes = get_active_votes(d.second.author, d.second.permlink);
                    }

                    _state.witness_schedule = my->database().get_witness_schedule_object();

                } catch (const fc::exception &e) {
                    _state.error = e.to_detail_string();
                }
                return _state;
            });
        }

            DEFINE_API(database_api,get_transaction){
                return my->database().with_read_lock([&]() {
                    CHECK_ARG_SIZE( 1 )
                    auto id = args[0].as< transaction_id_type >();
                    const auto &idx = my->database().get_index<operation_index>().indices().get<by_transaction_id>();
                    auto itr = idx.lower_bound(id);
                    if (itr != idx.end() && itr->trx_id == id) {
                        auto blk = my->database().fetch_block_by_number(itr->block);
                        FC_ASSERT(blk.valid());
                        FC_ASSERT(blk->transactions.size() > itr->trx_in_block);
                        annotated_signed_transaction result = blk->transactions[itr->trx_in_block];
                        result.block_num = itr->block;
                        result.transaction_num = itr->trx_in_block;
                        return result;
                    }
                    FC_ASSERT(false, "Unknown Transaction ${t}", ("t", id));
                });
        }

        template<
                typename Object,
                typename DatabaseIndex,
                typename DiscussionIndex,
                typename CommentIndex,
                typename ...Args
        >
        std::multimap<Object, discussion, DiscussionIndex> database_api::select(
                const std::set<std::string> &select_set,
                const discussion_query &query,
                comment_object::id_type parent,
                const std::function<bool(const comment_api_obj &)> &filter,
                const std::function<bool(const comment_api_obj &)> &exit,
                const std::function<bool(const Object &)> &exit2,
                Args... args) const {
            std::multimap<Object, discussion, DiscussionIndex> map_result;
            std::string helper;

            const auto &index = my->database().get_index<DatabaseIndex>().indices().template get<DiscussionIndex>();

            if (!select_set.empty()) {
                for (const auto &iterator : select_set) {
                    helper = fc::to_lower(iterator);

                    auto tidx_itr = index.lower_bound(boost::make_tuple(helper, args...));

                    auto result = get_discussions<Object, DatabaseIndex, DiscussionIndex, CommentIndex>(query, helper, parent, index, tidx_itr, filter, exit, exit2);

                    map_result.insert(result.cbegin(), result.cend());
                }
            } else {
                auto tidx_itr = index.lower_bound(boost::make_tuple(helper, args...));

                map_result = get_discussions<Object, DatabaseIndex, DiscussionIndex, CommentIndex>(query, helper, parent, index, tidx_itr, filter, exit, exit2);
            }

            return map_result;
        }

        DEFINE_API(database_api,get_reward_fund){
            return my->database().with_read_lock(
                    [&]() {
                        CHECK_ARG_SIZE( 1 )
                        string name = args[0].as< string >();
                        auto fund = my->database().find<reward_fund_object, by_name>(name);
                        FC_ASSERT(fund != nullptr, "Invalid reward fund name");
                        return *fund;
                    }
            );
        }

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Proposed transactions                                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////

            DEFINE_API(database_api,get_proposed_transactions){
                return my->get_proposed_transactions(name);
            }

/** TODO: add secondary index that will accelerate this process */
            DEFINE_API(database_api::database_api_impl,get_proposed_transactions){
                const auto &idx = database().get_index<proposal_index>();
                vector<proposal_object> result;

                idx.inspect_objects([&](const proposal_object &p) {
                    if (p.required_active_approvals.find(name) != p.required_active_approvals.end()) {
                        result.push_back(p);
                    } else if (p.required_owner_approvals.find(name) != p.required_owner_approvals.end()) {
                        result.push_back(p);
                    } else if (p.available_active_approvals.find(name) != p.available_active_approvals.end()) {
                        result.push_back(p);
                    }
                });
                return result;
        }
    }}
} // steemit::application
