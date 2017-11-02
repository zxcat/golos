#include <steemit/chain/objects/account_object.hpp>
#include <steemit/plugins/follow_api/follow_api.hpp>
#include <steemit/plugins/json_rpc/utility.hpp>
#include <steemit/plugins/chain/chain_plugin.hpp>
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>
#include <steemit/plugins/follow/follow_objects.hpp>

namespace steemit {
    namespace plugins {
        namespace follow_api {
            using namespace follow;

            inline void set_what(vector<follow_type> &what, uint16_t bitmask) {
                if (bitmask & 1 << blog) {
                    what.push_back(blog);
                }
                if (bitmask & 1 << ignore) {
                    what.push_back(ignore);
                }
            }

            struct follow_api::follow_api_impl final {
                follow_api_impl() : db_(appbase::app().get_plugin<chain_interface::chain_plugin>().db()) {}

                get_followers_r           get_followers           (const get_followers_a&);
                get_following_r           get_following           (const get_following_a&);
                get_feed_entries_r        get_feed_entries        (const get_feed_entries_a&);
                get_feed_r                get_feed                (const get_feed_a&);
                get_blog_entries_r        get_blog_entries        (const get_blog_entries_a&);
                get_blog_r                get_blog                (const get_blog_a&);
                get_account_reputations_r get_account_reputations (const get_account_reputations_a&);
                get_follow_count_r        get_follow_count        (const get_follow_count_a&);
                get_reblogged_by_r        get_reblogged_by        (const get_reblogged_by_a&);
                get_blog_authors_r        get_blog_authors        (const get_blog_authors_a&);

                steemit::chain::database &database() {
                    return db_;
                }

            private:
                steemit::chain::database &db_;
            };

            get_followers_r follow_api::follow_api_impl::get_followers(const get_followers_a&args) {

                FC_ASSERT(args.limit <= 1000);
                get_followers_r result;
                result.followers.reserve(args.limit);

                const auto &idx = database().get_index<follow_index>().indices().get<by_following_follower>();
                auto itr = idx.lower_bound(std::make_tuple(args.account, args.start));
                while (itr != idx.end() && result.followers.size() < args.limit && itr->following == args.account) {
                    if (args.type == undefined || itr->what & (1 << args.type)) {
                        follow_api_object entry;
                        entry.follower = itr->follower;
                        entry.following = itr->following;
                        set_what(entry.what, itr->what);
                        result.followers.push_back(entry);
                    }

                    ++itr;
                }

                return result;
            }

            get_following_r follow_api::follow_api_impl::get_following(const get_following_a&args) {
                FC_ASSERT(args.limit <= 100);
                get_following_r result;
                const auto &idx = database().get_index<follow_index>().indices().get<by_follower_following>();
                auto itr = idx.lower_bound(std::make_tuple(args.account, args.start));
                while (itr != idx.end() && result.following.size() < args.limit && itr->follower == args.account) {
                    if (args.type == undefined || itr->what & (1 << args.type)) {
                        follow_api_object entry;
                        entry.follower = itr->follower;
                        entry.following = itr->following;
                        set_what(entry.what, itr->what);
                        result.following.push_back(entry);
                    }

                    ++itr;
                }

                return result;
            }

            get_follow_count_r follow_api::follow_api_impl::get_follow_count(const get_follow_count_a&args) {
                get_follow_count_r result;
                auto itr = database().find<follow_count_object, by_account>(args.account);

                if (itr != nullptr) {
                    result = get_follow_count_r{itr->account, itr->follower_count, itr->following_count};
                } else {
                    result.account = args.account;
                }

                return result;
            }

            get_feed_entries_r follow_api::follow_api_impl::get_feed_entries(const get_feed_entries_a&args) {
                FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 feed entries at a time.");

                auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                get_feed_entries_r result;
                result.feed.reserve(args.limit);

                const auto &db = database();
                const auto &feed_idx = db.get_index<feed_index>().indices().get<by_feed>();
                auto itr = feed_idx.lower_bound(boost::make_tuple(args.account, entry_id));

                while (itr != feed_idx.end() && itr->account == args.account && result.feed.size() < args.limit) {
                    const auto &comment = db.get(itr->comment);
                    feed_entry entry;
                    entry.author = comment.author;
                    entry.permlink = to_string(comment.permlink);
                    entry.entry_id = itr->account_feed_id;
                    if (itr->first_reblogged_by != account_name_type()) {
                        entry.reblog_by.reserve(itr->reblogged_by.size());
                        for (const auto &a : itr->reblogged_by) {
                            entry.reblog_by.push_back(a);
                        }
                        //entry.reblog_by = itr->first_reblogged_by;
                        entry.reblog_on = itr->first_reblogged_on;
                    }
                    result.feed.push_back(entry);

                    ++itr;
                }

                return result;
            }

            get_feed_r follow_api::follow_api_impl::get_feed(const get_feed_a&args) {
                FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 feed entries at a time.");

                auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                get_feed_r result;
                result.feed.reserve(args.limit);

                const auto &db = database();
                const auto &feed_idx = db.get_index<feed_index>().indices().get<by_feed>();
                auto itr = feed_idx.lower_bound(boost::make_tuple(args.account, entry_id));

                while (itr != feed_idx.end() && itr->account == args.account && result.feed.size() < args.limit) {
                    const auto &comment = db.get(itr->comment);
                    comment_feed_entry entry;
                    entry.comment = comment;
                    entry.entry_id = itr->account_feed_id;
                    if (itr->first_reblogged_by != account_name_type()) {
                        //entry.reblog_by = itr->first_reblogged_by;
                        entry.reblog_by.reserve(itr->reblogged_by.size());
                        for (const auto &a : itr->reblogged_by) {
                            entry.reblog_by.push_back(a);
                        }
                        entry.reblog_on = itr->first_reblogged_on;
                    }
                    result.feed.push_back(entry);

                    ++itr;
                }

                return result;
            }

            get_blog_entries_r follow_api::follow_api_impl::get_blog_entries(const get_blog_entries_a&args) {
                FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 blog entries at a time.");

                auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                get_blog_entries_r result;
                result.blog.reserve(args.limit);

                const auto &db = database();
                const auto &blog_idx = db.get_index<blog_index>().indices().get<by_blog>();
                auto itr = blog_idx.lower_bound(boost::make_tuple(args.account, entry_id));

                while (itr != blog_idx.end() && itr->account == args.account && result.blog.size() < args.limit) {
                    const auto &comment = db.get(itr->comment);
                    blog_entry entry;
                    entry.author = comment.author;
                    entry.permlink = to_string(comment.permlink);
                    entry.blog = args.account;
                    entry.reblog_on = itr->reblogged_on;
                    entry.entry_id = itr->blog_feed_id;

                    result.blog.push_back(entry);

                    ++itr;
                }

                return result;
            }

            get_blog_r follow_api::follow_api_impl::get_blog(const get_blog_a&args) {
                FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 blog entries at a time.");

                auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                get_blog_r result;
                result.blog.reserve(args.limit);

                const auto &db = database();
                const auto &blog_idx = db.get_index<blog_index>().indices().get<by_blog>();
                auto itr = blog_idx.lower_bound(boost::make_tuple(args.account, entry_id));

                while (itr != blog_idx.end() && itr->account == args.account && result.blog.size() < args.limit) {
                    const auto &comment = db.get(itr->comment);
                    comment_blog_entry entry;
                    entry.comment = comment;
                    entry.blog = args.account;
                    entry.reblog_on = itr->reblogged_on;
                    entry.entry_id = itr->blog_feed_id;

                    result.blog.push_back(entry);

                    ++itr;
                }

                return result;
            }

            get_account_reputations_r follow_api::follow_api_impl::get_account_reputations(const get_account_reputations_a&args) {
                FC_ASSERT(args.limit <= 1000, "Cannot retrieve more than 1000 account reputations at a time.");

                const auto &acc_idx = database().get_index<account_index>().indices().get<by_name>();
                const auto &rep_idx = database().get_index<reputation_index>().indices().get<by_account>();

                auto acc_itr = acc_idx.lower_bound(args.account_lower_bound);

                get_account_reputations_r result;
                result.reputations.reserve(args.limit);

                while (acc_itr != acc_idx.end() && result.reputations.size() < args.limit) {
                    auto itr = rep_idx.find(acc_itr->name);
                    account_reputation rep;

                    rep.account = acc_itr->name;
                    rep.reputation = itr != rep_idx.end() ? itr->reputation : 0;

                    result.reputations.push_back(rep);

                    ++acc_itr;
                }

                return result;
            }

            get_reblogged_by_r follow_api::follow_api_impl::get_reblogged_by(const get_reblogged_by_a&args){
                auto &db = database();
                get_reblogged_by_r result;
                const auto &post = db.get_comment(args.author, args.permlink);
                const auto &blog_idx = db.get_index<blog_index, by_comment>();
                auto itr = blog_idx.lower_bound(post.id);
                while (itr != blog_idx.end() && itr->comment == post.id && result.accounts.size() < 2000) {
                    result.accounts.push_back(itr->account);
                    ++itr;
                }
                return result;
            }

            get_blog_authors_r follow_api::follow_api_impl::get_blog_authors(const get_blog_authors_a&args){
                auto &db = database();
                get_blog_authors_r result;
                const auto &stats_idx = db.get_index<blog_author_stats_index, by_blogger_guest_count>();
                auto itr = stats_idx.lower_bound(boost::make_tuple(args.blog_account));
                while (itr != stats_idx.end() && itr->blogger == args.blog_account && result.blog_authors.size()) {
                    result.blog_authors.emplace_back(itr->guest, itr->count);
                    ++itr;
                }
                return result;
            }


            follow_api::follow_api():my(new follow_api_impl){
                JSON_RPC_REGISTER_API(__name__);
            }
                DEFINE_API(follow_api, get_followers) {
                    auto tmp = args.args->at(0).as<get_followers_a>();
                    return my->database().with_read_lock([&]() {
                        return my->get_followers(tmp);
                    });
                }

                DEFINE_API(follow_api, get_following) {
                    auto tmp = args.args->at(0).as<get_following_a>();
                    return my->database().with_read_lock([&]() {
                        return my->get_following(tmp);
                    });
                }

                DEFINE_API(follow_api, get_follow_count) {
                    auto tmp = args.args->at(0).as<get_follow_count_a>();
                    return my->database().with_read_lock([&]() {
                        return my->get_follow_count(tmp);
                    });
                }

                DEFINE_API(follow_api, get_feed_entries) {
                    auto tmp = args.args->at(0).as<get_feed_entries_a>();
                    return my->database().with_read_lock([&]() {
                        return my->get_feed_entries(tmp);
                    });
                }

                DEFINE_API(follow_api, get_feed) {
                    auto tmp = args.args->at(0).as<get_feed_a>();
                    return my->database().with_read_lock([&]() {
                        return my->get_feed(tmp);
                    });
                }

                DEFINE_API(follow_api, get_blog_entries) {
                    auto tmp = args.args->at(0).as<get_blog_entries_a>();
                    return my->database().with_read_lock([&]() {
                        return my->get_blog_entries(tmp);
                    });
                }

                DEFINE_API(follow_api, get_blog) {
                    auto tmp = args.args->at(0).as<get_blog_a>();
                    return my->database().with_read_lock(
                        [&]() {
                            return my->get_blog(tmp);
                        }
                    );
                }

                DEFINE_API(follow_api, get_account_reputations) {
                    auto tmp = args.args->at(0).as<get_account_reputations_a>();
                    return my->database().with_read_lock(
                            [&]() {
                                return my->get_account_reputations(tmp);
                            }
                    );
                }

                DEFINE_API(follow_api, get_reblogged_by) {
                    auto tmp = args.args->at(0).as<get_reblogged_by_a>();
                    return my->database().with_read_lock(
                            [&]() {
                                return my->get_reblogged_by(tmp);
                            }
                    );
                }

                DEFINE_API(follow_api, get_blog_authors) {
                    auto tmp = args.args->at(0).as<get_blog_authors_a>();
                    return my->database().with_read_lock(
                            [&]() {
                                return my->get_blog_authors(tmp);
                            }
                    );
                }

    get_followers_r follow_api::get_followers_native(const get_followers_a& args) {
            return my->database().with_read_lock([&]() {
            return my->get_followers(args);
        });
    }

    get_following_r follow_api::get_following_native(const get_following_a&args){
        return my->database().with_read_lock([&]() {
            return my->get_following(args);
        });
    }

    get_follow_count_r follow_api::get_follow_count_native(const get_follow_count_a& args){
        return my->database().with_read_lock([&]() {
            return my->get_follow_count(args);
        });
    }

    get_feed_entries_r follow_api::get_feed_entries_native(const get_feed_entries_a&args){
        return my->database().with_read_lock([&]() {
            return my->get_feed_entries(args);
        });
    }

    get_feed_r follow_api::get_feed_native(const get_feed_a&args){
        return my->database().with_read_lock([&]() {
            return my->get_feed(args);
        });
    }

    get_blog_entries_r follow_api::get_blog_entries_native(const get_blog_entries_a&args){
        return my->database().with_read_lock([&]() {
            return my->get_blog_entries(args);
        });
    }

    get_blog_r follow_api::get_blog_native(const get_blog_a&args){
        return my->database().with_read_lock(
                [&]() {
                    return my->get_blog(args);
                }
        );
    }

    get_account_reputations_r follow_api::get_account_reputations_native(const get_account_reputations_a&args){
        return my->database().with_read_lock(
                [&]() {
                    return my->get_account_reputations(args);
                }
        );
    }

    get_reblogged_by_r follow_api::get_reblogged_by_native(const get_reblogged_by_a& args){
        return my->database().with_read_lock(
                [&]() {
                    return my->get_reblogged_by(args);
                }
        );
    }

    get_blog_authors_r follow_api::get_blog_authors_native(const get_blog_authors_a& args){
        return my->database().with_read_lock(
                [&]() {
                    return my->get_blog_authors(args);
                }
        );
    }

            }
        }
    } // steemit::follow_api