#include <steemit/plugins/follow/follow_objects.hpp>
#include <steemit/plugins/follow/follow_operations.hpp>

#include <steemit/protocol/config.hpp>

#include <steemit/chain/database.hpp>
#include <steemit/chain/generic_custom_operation_interpreter.hpp>
#include <steemit/chain/operation_notification.hpp>
#include <steemit/chain/account_object.hpp>
#include <steemit/chain/comment_object.hpp>

#include <fc/smart_ref_impl.hpp>

#include <memory>

namespace steemit {
    namespace plugins {
        namespace follow {
                using namespace steemit::protocol;

            struct pre_operation_visitor {
                follow_plugin &_plugin;
                chain::database &db;

                pre_operation_visitor(follow_plugin &plugin, steemit::chain::database &db) : _plugin(plugin),db(db) {}

                typedef void result_type;

                template<typename T>
                void operator()(const T &) const {
                }

                void operator()(const vote_operation &op) const {
                    try {

                        const auto &c = db.get_comment(op.author, op.permlink);

                        if (db.calculate_discussion_payout_time(c) == fc::time_point_sec::maximum()) {
                            return;
                        }

                        const auto &cv_idx = db.get_index<comment_vote_index>().indices().get<by_comment_voter>();
                        auto cv = cv_idx.find(std::make_tuple(c.id, db.get_account(op.voter).id));

                        if (cv != cv_idx.end()) {
                            const auto &rep_idx = db.get_index<reputation_index>().indices().get<by_account>();
                            auto rep = rep_idx.find(op.author);

                            if (rep != rep_idx.end()) {
                                db.modify(*rep, [&](reputation_object &r) {
                                    r.reputation -= (cv->rshares >> 6); // Shift away precision from vests. It is noise
                                });
                            }
                        }
                    }
                    catch (const fc::exception &e) {
                    }
                }

                void operator()(const delete_comment_operation &op) const {
                    try {
                        const auto *comment = db.find_comment(op.author, op.permlink);

                        if (comment == nullptr) {
                            return;
                        }
                        if (comment->parent_author.size()) {
                            return;
                        }

                        const auto &feed_idx = db.get_index<feed_index>().indices().get<by_comment>();
                        auto itr = feed_idx.lower_bound(comment->id);

                        while (itr != feed_idx.end() &&
                               itr->comment == comment->id) {
                            const auto &old_feed = *itr;
                            ++itr;
                            db.remove(old_feed);
                        }

                        const auto &blog_idx = db.get_index<blog_index>().indices().get<by_comment>();
                        auto blog_itr = blog_idx.lower_bound(comment->id);

                        while (blog_itr != blog_idx.end() &&
                               blog_itr->comment == comment->id) {
                            const auto &old_blog = *blog_itr;
                            ++blog_itr;
                            db.remove(old_blog);
                        }
                    } FC_CAPTURE_AND_RETHROW()
                }
            };

            struct post_operation_visitor {
                follow_plugin &_plugin;
                chain::database &db;

                post_operation_visitor(follow_plugin &plugin,chain::database &db)
                        : _plugin(plugin),db(db) {
                }

                typedef void result_type;

                template<typename T>
                void operator()(const T &) const {
                }

                void operator()(const custom_json_operation &op) const {
                    try {
                        if (op.id == follow_plugin::__name__) {
                            custom_json_operation new_cop;

                            new_cop.required_auths = op.required_auths;
                            new_cop.required_posting_auths = op.required_posting_auths;
                            new_cop.id = follow_plugin::__name__;
                            follow_operation fop;

                            try {
                                fop = fc::json::from_string(op.json).as<follow_operation>();
                            }
                            catch (const fc::exception &) {
                                return;
                            }

                            auto new_fop = follow_plugin_operation(fop);
                            new_cop.json = fc::json::to_string(new_fop);
                            std::shared_ptr<custom_operation_interpreter> eval = db.get_custom_json_evaluator(op.id);
                            eval->apply(new_cop);
                        }
                    } FC_CAPTURE_AND_RETHROW()
                }

                void operator()(const comment_operation &op) const {
                    try {
                        if (op.parent_author.size() > 0) {
                            return;
                        }

                        const auto &c = db.get_comment(op.author, op.permlink);

                        if (c.created != db.head_block_time()) {
                            return;
                        }

                        const auto &idx = db.get_index<follow_index>().indices().get<by_following_follower>();
                        const auto &comment_idx = db.get_index<feed_index>().indices().get<by_comment>();
                        auto itr = idx.find(op.author);

                        const auto &feed_idx = db.get_index<feed_index>().indices().get<by_feed>();

                        while (itr != idx.end() &&
                               itr->following == op.author) {
                            if (itr->what & (1 << blog)) {
                                uint32_t next_id = 0;
                                auto last_feed = feed_idx.lower_bound(itr->follower);

                                if (last_feed != feed_idx.end() &&
                                    last_feed->account == itr->follower) {
                                    next_id = last_feed->account_feed_id + 1;
                                }

                                if (comment_idx.find(boost::make_tuple(c.id, itr->follower)) ==
                                    comment_idx.end()) {
                                    db.create<feed_object>([&](feed_object &f) {
                                        f.account = itr->follower;
                                        f.comment = c.id;
                                        f.account_feed_id = next_id;
                                    });

                                    const auto &old_feed_idx = db.get_index<feed_index>().indices().get<by_old_feed>();
                                    auto old_feed = old_feed_idx.lower_bound(itr->follower);

                                    while (old_feed->account == itr->follower &&
                                           next_id - old_feed->account_feed_id > _plugin.max_feed_size()) {
                                        db.remove(*old_feed);
                                        old_feed = old_feed_idx.lower_bound(itr->follower);
                                    }
                                }
                            }

                            ++itr;
                        }

                        const auto &blog_idx = db.get_index<blog_index>().indices().get<by_blog>();
                        const auto &comment_blog_idx = db.get_index<blog_index>().indices().get<by_comment>();
                        auto last_blog = blog_idx.lower_bound(op.author);
                        uint32_t next_id = 0;

                        if (last_blog != blog_idx.end() &&
                            last_blog->account == op.author) {
                            next_id = last_blog->blog_feed_id + 1;
                        }

                        if (comment_blog_idx.find(boost::make_tuple(c.id, op.author)) ==
                            comment_blog_idx.end()) {
                            db.create<blog_object>([&](blog_object &b) {
                                b.account = op.author;
                                b.comment = c.id;
                                b.blog_feed_id = next_id;
                            });

                            const auto &old_blog_idx = db.get_index<blog_index>().indices().get<by_old_blog>();
                            auto old_blog = old_blog_idx.lower_bound(op.author);

                            while (old_blog->account == op.author &&
                                   next_id - old_blog->blog_feed_id >
                                   _plugin.max_feed_size()) {
                                db.remove(*old_blog);
                                old_blog = old_blog_idx.lower_bound(op.author);
                            }
                        }
                    } FC_LOG_AND_RETHROW()
                }

                void operator()(const vote_operation &op) const {
                    try {
                        const auto &comment = db.get_comment(op.author, op.permlink);

                        if (db.calculate_discussion_payout_time(comment) ==
                            fc::time_point_sec::maximum()) {
                            return;
                        }

                        const auto &cv_idx = db.get_index<comment_vote_index>().indices().get<by_comment_voter>();
                        auto cv = cv_idx.find(boost::make_tuple(comment.id, db.get_account(op.voter).id));

                        const auto &rep_idx = db.get_index<reputation_index>().indices().get<by_account>();
                        auto voter_rep = rep_idx.find(op.voter);
                        auto author_rep = rep_idx.find(op.author);

                        // Rules are a plugin, do not effect consensus, and are subject to change.
                        // Rule #1: Must have non-negative reputation to effect another user's reputation
                        if (voter_rep != rep_idx.end() &&
                            voter_rep->reputation < 0) {
                            return;
                        }

                        if (author_rep == rep_idx.end()) {
                            // Rule #2: If you are down voting another user, you must have more reputation than them to impact their reputation
                            // User rep is 0, so requires voter having positive rep
                            if (cv->rshares < 0 &&
                                !(voter_rep != rep_idx.end() &&
                                  voter_rep->reputation > 0)) {
                                return;
                            }

                            db.create<reputation_object>([&](reputation_object &r) {
                                r.account = op.author;
                                r.reputation = (cv->rshares
                                        >> 6); // Shift away precision from vests. It is noise
                            });
                        } else {
                            // Rule #2: If you are down voting another user, you must have more reputation than them to impact their reputation
                            if (cv->rshares < 0 &&
                                !(voter_rep != rep_idx.end() &&
                                  voter_rep->reputation >
                                  author_rep->reputation)) {
                                return;
                            }

                            db.modify(*author_rep, [&](reputation_object &r) {
                                r.reputation += (cv->rshares >> 6); // Shift away precision from vests. It is noise
                            });
                        }
                    } FC_CAPTURE_AND_RETHROW()
                }
            };

                struct follow_plugin::follow_plugin_impl {
                public:
                    follow_plugin_impl() : database_(appbase::app().get_plugin<steemit::plugins::chain::chain_plugin>().db()) {}

                    ~follow_plugin_impl(){};

                    void plugin_initialize(follow_plugin& self){
                        // Each plugin needs its own evaluator registry.
                        _custom_operation_interpreter = std::make_shared<generic_custom_operation_interpreter < follow_plugin_operation>>(database());

                        // Add each operation evaluator to the registry
                        _custom_operation_interpreter->register_evaluator<follow_evaluator>(&self);
                        _custom_operation_interpreter->register_evaluator<reblog_evaluator>(&self);

                        // Add the registry to the database so the database can delegate custom ops to the plugin
                        database().set_custom_operation_interpreter(__name__, _custom_operation_interpreter);
                    }

                    steemit::chain::database &database() {
                        return database_;
                    }


                    void pre_operation(const operation_notification &op_obj,follow_plugin&self){
                        try {
                            op_obj.op.visit(pre_operation_visitor(self,database()));
                        }
                        catch (const fc::assert_exception &) {
                            if (database().is_producing()) {
                                throw;
                            }
                        }
                    }

                    void post_operation(const operation_notification &op_obj,follow_plugin&self){
                        try {
                            op_obj.op.visit(post_operation_visitor(self,database()));
                        }
                        catch (fc::assert_exception) {
                            if (database().is_producing()) {
                                throw;
                            }
                        }
                    }

                    chain::database &database_;

                    uint32_t max_feed_size_ = 500;

                    std::shared_ptr<generic_custom_operation_interpreter <follow::follow_plugin_operation>> _custom_operation_interpreter;
                };

            follow_plugin::follow_plugin() {
                name(__name__);
            }

            void follow_plugin::set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg
            ) {
                cli.add_options()
                        ("follow-max-feed-size", boost::program_options::value<uint32_t>()->default_value(500), "Set the maximum size of cached feed for an account");
                cfg.add(cli);
            }

            void follow_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                try {
                    ilog("Intializing follow plugin");
                    my.reset(new follow_plugin_impl());
                    chain::database &db = my->database();
                    my->plugin_initialize(*this);

                    db.pre_apply_operation.connect([&](const operation_notification &o) { my->pre_operation(o,*this); });
                    db.post_apply_operation.connect([&](const operation_notification &o) { my->post_operation(o,*this); });
                    db.add_plugin_index<follow_index>();
                    db.add_plugin_index<feed_index>();
                    db.add_plugin_index<blog_index>();
                    db.add_plugin_index<reputation_index>();
                    db.add_plugin_index<follow_count_index>();
                    db.add_plugin_index<blog_author_stats_index>();

                    if (options.count("follow-max-feed-size")) {
                        uint32_t feed_size = options["follow-max-feed-size"].as<uint32_t>();
                        my->max_feed_size_ = feed_size;
                    }
                } FC_CAPTURE_AND_RETHROW()
            }

            void follow_plugin::plugin_startup() {}

            uint32_t follow_plugin::max_feed_size() {
                return my->max_feed_size_;
            }

            follow_plugin::~follow_plugin() {

            }

        }
    }
} // steemit::follow
