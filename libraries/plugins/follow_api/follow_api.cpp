#include <steemit/chain/account_object.hpp>
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

                struct follow_api::follow_api_impl {
                    follow_api_impl()
                            : db_(appbase::app().get_plugin<steemit::plugins::chain::chain_plugin>().db()) {}

                        DECLARE_API(
                                (get_followers)
                                (get_following)
                                (get_feed_entries)
                                (get_feed)
                                (get_blog_entries)
                                (get_blog)
                                (get_account_reputations)
                                (get_follow_count)
                                (get_reblogged_by)
                                (get_blog_authors)
                        )

                        steemit::chain::database& database(){
                            return db_;
                        }

                        steemit::chain::database &db_;
                    };

                    DEFINE_API(follow_api::follow_api_impl,get_followers){
                        FC_ASSERT( args.limit <= 1000 );
                        get_followers_return result;
                        result.followers.reserve(args.limit);

                        const auto &idx =database().get_index<follow_index>().indices().get<by_following_follower>();
                        auto itr = idx.lower_bound(std::make_tuple(args.account, args.start ));
                        while( itr != idx.end() && result.followers.size() < args.limit && itr->following == args.account ){
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

                    DEFINE_API(follow_api::follow_api_impl,get_following) {
                        FC_ASSERT(args.limit <= 100);
                        get_following_return result;
                        const auto &idx = database().get_index<follow_index>().indices().get<by_follower_following>();
                        auto itr = idx.lower_bound(std::make_tuple(args.account, args.start));
                        while( itr != idx.end() && result.following.size() < args.limit && itr->follower == args.account ){
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

                    DEFINE_API(follow_api::follow_api_impl,get_follow_count){
                        get_follow_count_return result;
                        auto itr = database().find<follow_count_object, by_account>(args.account);

                        if (itr != nullptr) {
                            result = get_follow_count_return{ itr->account, itr->follower_count, itr->following_count };
                        } else {
                            result.account = args.account;
                        }

                        return result;
                    }

                    DEFINE_API(follow_api::follow_api_impl,get_feed_entries) {
                        FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 feed entries at a time.");

                        auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                        get_feed_entries_return result;
                        result.feed.reserve( args.limit );

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
                            result.feed.push_back( entry );

                            ++itr;
                        }

                        return result;
                    }

                    DEFINE_API(follow_api::follow_api_impl,get_feed){
                        FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 feed entries at a time.");

                        auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                        get_feed_return result;
                        result.feed.reserve( args.limit );

                        const auto &db = database();
                        const auto &feed_idx = db.get_index<feed_index>().indices().get<by_feed>();
                        auto itr = feed_idx.lower_bound(boost::make_tuple( args.account, entry_id));

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

                    DEFINE_API(follow_api::follow_api_impl,get_blog_entries){
                        FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 blog entries at a time.");

                        auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                        get_blog_entries_return result;
                        result.blog.reserve( args.limit );

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

                    DEFINE_API(follow_api::follow_api_impl,get_blog){
                        FC_ASSERT(args.limit <= 500, "Cannot retrieve more than 500 blog entries at a time.");

                        auto entry_id = args.start_entry_id == 0 ? args.start_entry_id : ~0;

                        get_blog_return result;
                        result.blog.reserve( args.limit );

                        const auto &db = database();
                        const auto &blog_idx = db.get_index<blog_index>().indices().get<by_blog>();
                        auto itr = blog_idx.lower_bound(boost::make_tuple(args.account, entry_id ));

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

                    DEFINE_API(follow_api::follow_api_impl,get_account_reputations) {
                        FC_ASSERT(args.limit <= 1000, "Cannot retrieve more than 1000 account reputations at a time.");

                        const auto &acc_idx = database().get_index<account_index>().indices().get<by_name>();
                        const auto &rep_idx = database().get_index<reputation_index>().indices().get<by_account>();

                        auto acc_itr = acc_idx.lower_bound(args.account_lower_bound);

                        get_account_reputations_return result;
                        result.reputations.reserve( args.limit );

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

                    DEFINE_API(follow_api::follow_api_impl,get_reblogged_by){
                            auto &db = database();
                            get_reblogged_by_return result;
                            const auto &post = db.get_comment(args.author, args.permlink);
                            const auto &blog_idx = db.get_index<blog_index, by_comment>();
                            auto itr = blog_idx.lower_bound(post.id);
                            while (itr != blog_idx.end() && itr->comment == post.id && result.accounts.size() < 2000) {
                                result.accounts.push_back(itr->account);
                                ++itr;
                            }
                            return result;
                    }

                    DEFINE_API(follow_api::follow_api_impl,get_blog_authors){
                            auto &db = database();
                            get_blog_authors_return result;
                            const auto &stats_idx = db.get_index<blog_author_stats_index, by_blogger_guest_count>();
                            auto itr = stats_idx.lower_bound(boost::make_tuple(args.blog_account));
                            while (itr != stats_idx.end() && itr->blogger == args.blog_account && result.blog_authors.size()) {
                                result.blog_authors.emplace_back(itr->guest, itr->count);
                                ++itr;
                            }
                            return result;
                    }


                follow_api::follow_api() {
                    my = std::make_shared<follow_api_impl>();

                    JSON_RPC_REGISTER_API(
                            __name__,
                            (get_followers)
                                    (get_following)
                                    (get_follow_count)
                                    (get_feed_entries)
                                    (get_feed)
                                    (get_blog_entries)
                                    (get_blog)
                                    (get_account_reputations)
                                    (get_reblogged_by)
                                    (get_blog_authors)
                    )
                }


                DEFINE_API(follow_api,get_followers){
                    return my->database().with_read_lock([&]() {
                        return my->get_followers(args);
                    });
                }

                DEFINE_API(follow_api,get_following) {
                    return my->database().with_read_lock([&]() {
                        return my->get_following(args);
                    });
                }

                DEFINE_API(follow_api,get_follow_count) {
                    return my->database().with_read_lock([&]() {
                        return my->get_follow_count(args);
                    });
                }

                DEFINE_API(follow_api,get_feed_entries){
                    return my->database().with_read_lock([&]() {
                        return my->get_feed_entries(args);
                    });
                }

                DEFINE_API(follow_api,get_feed){
                    return my->database().with_read_lock([&]() {
                        return my->get_feed(args);
                    });
                }

                DEFINE_API(follow_api,get_blog_entries){
                    return my->database().with_read_lock([&]() {
                        return my->get_blog_entries(args);
                    });
                }

                DEFINE_API(follow_api,get_blog){
                    return my->database().with_read_lock([&]() {
                        return my->get_blog(args);
                    });
                }

                DEFINE_API(follow_api,get_account_reputations){
                    return my->database().with_read_lock([&]() {
                        return my->get_account_reputations(args);
                    });
                }

                DEFINE_API( follow_api, get_reblogged_by ) {
                    return my->database().with_read_lock(
                            [&]() {
                                return my->get_reblogged_by( args );
                            }
                    );
                }

                DEFINE_API( follow_api, get_blog_authors ) {
                    return my->database().with_read_lock(
                            [&](){
                                return my->get_blog_authors( args );
                            }
                    );
                }

            }
        }
    } // steemit::follow_api

