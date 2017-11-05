#pragma once

#include <golos/plugins/database_api/api_objects/comment_api_object.hpp>
#include <golos/plugins/json_rpc/utility.hpp>
#include <golos/plugins/follow/follow_objects.hpp>

namespace golos {
    namespace plugins {
        namespace follow {

            using namespace chain;
            using json_rpc::msg_pack;

            struct feed_entry {
                account_name_type author;
                string permlink;
                vector<account_name_type> reblog_by;
                time_point_sec reblog_on;
                uint32_t entry_id = 0;
            };

            struct comment_feed_entry {
                database_api::comment_api_object comment;
                vector<account_name_type> reblog_by;
                time_point_sec reblog_on;
                uint32_t entry_id = 0;
            };

            struct blog_entry {
                account_name_type author;
                account_name_type permlink;
                account_name_type blog;
                time_point_sec reblog_on;
                uint32_t entry_id = 0;
            };

            struct comment_blog_entry {
                database_api::comment_api_object comment;
                string blog;
                time_point_sec reblog_on;
                uint32_t entry_id = 0;
            };

            struct account_reputation {
                account_name_type account;
                golos::protocol::share_type reputation;
            };

            struct follow_api_object {
                account_name_type follower;
                account_name_type following;
                vector<follow::follow_type> what;
            };

            struct reblog_count {
                account_name_type author;
                uint32_t count;
            };

            struct get_followers_a {
                account_name_type account;
                account_name_type start;
                follow::follow_type type;
                uint32_t limit = 1000;
            };

            struct get_followers_r {
                vector<follow_api_object> followers;
            };

            typedef get_followers_a get_following_a;

            struct get_following_r {
                vector<follow_api_object> following;
            };

            struct get_follow_count_a {
                account_name_type account;
            };

            struct get_follow_count_r {
                account_name_type account;
                uint32_t follower_count;
                uint32_t following_count;
            };

            struct get_feed_entries_a {
                account_name_type account;
                uint32_t start_entry_id = 0;
                uint32_t limit = 500;
            };

            struct get_feed_entries_r {
                vector<feed_entry> feed;
            };

            typedef get_feed_entries_a get_feed_a;

            struct get_feed_r {
                vector<comment_feed_entry> feed;
            };

            typedef get_feed_entries_a get_blog_entries_a;

            struct get_blog_entries_r {
                vector<blog_entry> blog;
            };

            typedef get_feed_entries_a get_blog_a;

            struct get_blog_r {
                vector<comment_blog_entry> blog;
            };

            struct get_account_reputations_a {
                account_name_type account_lower_bound;
                uint32_t limit = 1000;
            };

            struct get_account_reputations_r {
                vector<account_reputation> reputations;
            };

            struct get_reblogged_by_a {
                account_name_type author;
                string permlink;
            };

            struct get_reblogged_by_r {
                vector<account_name_type> accounts;
            };

            struct get_blog_authors_a {
                account_name_type blog_account;
            };

            struct get_blog_authors_r {
                vector<std::pair<account_name_type, uint32_t> > blog_authors;
            };


            ///               API,                          args,       return
            DEFINE_API_ARGS(get_followers, msg_pack, get_followers_r)
            DEFINE_API_ARGS(get_following, msg_pack, get_following_r)
            DEFINE_API_ARGS(get_follow_count, msg_pack, get_follow_count_r)
            DEFINE_API_ARGS(get_feed_entries, msg_pack, get_feed_entries_r)
            DEFINE_API_ARGS(get_feed, msg_pack, get_feed_r)
            DEFINE_API_ARGS(get_blog_entries, msg_pack, get_blog_entries_r)
            DEFINE_API_ARGS(get_blog, msg_pack, get_blog_r)
            DEFINE_API_ARGS(get_account_reputations, msg_pack, get_account_reputations_r)
            DEFINE_API_ARGS(get_reblogged_by, msg_pack, get_reblogged_by_r)
            DEFINE_API_ARGS(get_blog_authors, msg_pack, get_blog_authors_r)

            class api final {
            public:
                constexpr static const char *plugin_name = "follow";

                api();

                ~api() = default;

                DECLARE_API (
                        (get_followers)(get_following)(get_follow_count)(get_feed_entries)(get_feed)(get_blog_entries)(
                                get_blog)(get_account_reputations)
                                ///Gets list of accounts that have reblogged a particular post
                                (get_reblogged_by)
                                /// Gets a list of authors that have had their content reblogged on a given blog account
                                (get_blog_authors))

                get_followers_r get_followers_native(const get_followers_a &);

                get_following_r get_following_native(const get_following_a &);

                get_feed_entries_r get_feed_entries_native(const get_feed_entries_a &);

                get_feed_r get_feed_native(const get_feed_a &);

                get_blog_entries_r get_blog_entries_native(const get_blog_entries_a &);

                get_blog_r get_blog_native(const get_blog_a &);

                get_account_reputations_r get_account_reputations_native(const get_account_reputations_a &);

                get_follow_count_r get_follow_count_native(const get_follow_count_a &);

                get_reblogged_by_r get_reblogged_by_native(const get_reblogged_by_a &);

                get_blog_authors_r get_blog_authors_native(const get_blog_authors_a &);


            private:
                struct api_impl;

                std::shared_ptr<api_impl> my;
            };

        }
    }
} // golos::follow

FC_REFLECT((golos::plugins::follow::feed_entry), (author)(permlink)(reblog_by)(reblog_on)(entry_id));

FC_REFLECT((golos::plugins::follow::comment_feed_entry), (comment)(reblog_by)(reblog_on)(entry_id));

FC_REFLECT((golos::plugins::follow::blog_entry), (author)(permlink)(blog)(reblog_on)(entry_id));

FC_REFLECT((golos::plugins::follow::comment_blog_entry), (comment)(blog)(reblog_on)(entry_id));

FC_REFLECT((golos::plugins::follow::account_reputation), (account)(reputation));

FC_REFLECT((golos::plugins::follow::follow_api_object), (follower)(following)(what));

FC_REFLECT((golos::plugins::follow::reblog_count), (author)(count));

FC_REFLECT((golos::plugins::follow::get_followers_a), (account)(start)(type)(limit));

FC_REFLECT((golos::plugins::follow::get_followers_r), (followers));

FC_REFLECT((golos::plugins::follow::get_following_r), (following));

FC_REFLECT((golos::plugins::follow::get_follow_count_a), (account));

FC_REFLECT((golos::plugins::follow::get_follow_count_r), (account)(follower_count)(following_count));

FC_REFLECT((golos::plugins::follow::get_feed_entries_a), (account)(start_entry_id)(limit));

FC_REFLECT((golos::plugins::follow::get_feed_entries_r), (feed));

FC_REFLECT((golos::plugins::follow::get_feed_r), (feed));

FC_REFLECT((golos::plugins::follow::get_blog_entries_r), (blog));

FC_REFLECT((golos::plugins::follow::get_blog_r), (blog));

FC_REFLECT((golos::plugins::follow::get_account_reputations_a), (account_lower_bound)(limit));

FC_REFLECT((golos::plugins::follow::get_account_reputations_r), (reputations));

FC_REFLECT((golos::plugins::follow::get_reblogged_by_a), (author)(permlink));

FC_REFLECT((golos::plugins::follow::get_reblogged_by_r), (accounts));

FC_REFLECT((golos::plugins::follow::get_blog_authors_a), (blog_account));

FC_REFLECT((golos::plugins::follow::get_blog_authors_r), (blog_authors));
