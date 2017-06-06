#include <steemit/languages/languages_plugin.hpp>

#include <steemit/application/impacted.hpp>

#include <steemit/protocol/config.hpp>

#include <steemit/chain/database.hpp>
#include <steemit/chain/hardfork.hpp>
#include <steemit/chain/index.hpp>
#include <steemit/chain/operation_notification.hpp>
#include <steemit/chain/account_object.hpp>
#include <steemit/chain/comment_object.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/json.hpp>
#include <fc/string.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

namespace steemit {
    namespace languages {

        namespace detail {

            using namespace steemit::protocol;

            class languages_plugin_impl final {
            public:
                languages_plugin_impl(languages_plugin &_plugin)
                        : _self(_plugin) {
                }

                virtual ~languages_plugin_impl();

                languages_plugin_impl &self() {
                    return *this;
                }

                steemit::chain::database &database() {
                    return _self.database();
                }

                void on_operation(const operation_notification &note);

                languages_plugin &_self;
                set<string> cache_languages;
            };

            languages_plugin_impl::~languages_plugin_impl() {
                return;
            }

            struct operation_visitor {
                operation_visitor(languages_plugin_impl &self) : languages_plugin_impl(self), _db(self.database()) {

                };
                typedef void result_type;

                languages_plugin_impl &languages_plugin_impl;
                steemit::chain::database &_db;

                void remove_stats(const language_object &tag, const language_stats_object &stats) const {
                    _db.modify(stats, [&](language_stats_object &s) {
                        if (tag.parent == comment_id_type()) {
                            s.total_children_rshares2 -= tag.children_rshares2;
                            s.top_posts--;
                        } else {
                            s.comments--;
                        }
                        s.net_votes -= tag.net_votes;
                    });
                }

                void add_stats(const language_object &tag, const language_stats_object &stats) const {
                    _db.modify(stats, [&](language_stats_object &s) {
                        if (tag.parent == comment_id_type()) {
                            s.total_children_rshares2 += tag.children_rshares2;
                            s.top_posts++;
                        } else {
                            s.comments++;
                        }
                        s.net_votes += tag.net_votes;
                    });
                }

                void remove_tag(const language_object &tag) const {
                    /// TODO: update tag stats object
                    _db.remove(tag);

                    const auto &idx = _db.get_index<author_language_stats_index>().indices().get<by_author_tag_posts>();
                    auto itr = idx.lower_bound(boost::make_tuple(tag.author, tag.language));
                    if (itr != idx.end() && itr->author == tag.author &&
                        itr->language == tag.language) {
                        _db.modify(*itr, [&](author_language_stats_object &stats) {
                            stats.total_posts--;
                        });
                    }
                }

                const language_stats_object &get_stats(const string &tag) const {
                    const auto &stats_idx = _db.get_index<language_stats_index>().indices().get<by_tag>();
                    auto itr = stats_idx.find(tag);
                    if (itr != stats_idx.end()) {
                        return *itr;
                    }

                    return _db.create<language_stats_object>([&](language_stats_object &stats) {
                        stats.language = tag;
                    });
                }

                void update_tag(
                        const language_object &current,
                        const comment_object &comment,
                        double hot,
                        double trending
                ) const {
                    const auto &stats = get_stats(current.language);
                    remove_stats(current, stats);

                    if (comment.mode != archived) {
                        _db.modify(current, [&](language_object &obj) {
                            obj.active = comment.active;
                            obj.cashout = comment.cashout_time;
                            obj.children = comment.children;
                            obj.net_rshares = comment.net_rshares.value;
                            obj.net_votes = comment.net_votes;
                            obj.children_rshares2 = comment.children_rshares2;
                            obj.hot = hot;
                            obj.trending = trending;
                            obj.mode = comment.mode;
                            if (obj.mode != first_payout) {
                                obj.promoted_balance = 0;
                            }
                        });
                        add_stats(current, stats);
                    } else {
                        _db.remove(current);
                    }
                }

                void create_tag(const string &language, const comment_object &comment, double hot, double trending) const {
                    comment_id_type parent;
                    account_id_type author = _db.get_account(comment.author).id;

                    if (comment.parent_author.size()) {
                        parent = _db.get_comment(comment.parent_author, comment.parent_permlink).id;
                    }

                    const auto &tag_obj = _db.create<language_object>([&](language_object &obj) {
                        obj.language = language;
                        obj.comment = comment.id;
                        obj.parent = parent;
                        obj.created = comment.created;
                        obj.active = comment.active;
                        obj.cashout = comment.cashout_time;
                        obj.net_votes = comment.net_votes;
                        obj.children = comment.children;
                        obj.net_rshares = comment.net_rshares.value;
                        obj.children_rshares2 = comment.children_rshares2;
                        obj.author = author;
                        obj.mode = comment.mode;
                        obj.hot = hot;
                        obj.trending = trending;
                    });
                    add_stats(tag_obj, get_stats(language));


                    const auto &idx = _db.get_index<author_language_stats_index>().indices().get<by_author_tag_posts>();
                    auto itr = idx.lower_bound(boost::make_tuple(author, language));
                    if (itr != idx.end() && itr->author == author &&
                        itr->language == language) {
                        _db.modify(*itr, [&](author_language_stats_object &stats) {
                            stats.total_posts++;
                        });
                    } else {
                        _db.create<author_language_stats_object>([&](author_language_stats_object &stats) {
                            stats.author = author;
                            stats.language = language;
                            stats.total_posts = 1;
                        });
                    }

                    languages_plugin_impl.self().cache_languages.emplace(language);
                }

                string filter_tags(const comment_object &c) const {
                    string language;
                    if (c.languages != "") {
                        language = fc::to_lower(to_string(c.languages));
                    }

                    return language;
                }

                /**
                 * https://medium.com/hacking-and-gonzo/how-reddit-ranking-algorithms-work-ef111e33d0d9#.lcbj6auuw
                 */
                template<int64_t S, int32_t T>
                double calculate_score(const share_type &score, const time_point_sec &created) const {
                    auto mod_score = score.value / S;

                    /// reddit algorithm
                    double order = log10(std::max<int64_t>(std::abs(mod_score), 1));

                    int sign = 0;
                    if (mod_score > 0) {
                        sign = 1;
                    } else if (mod_score < 0) {
                        sign = -1;
                    }

                    return sign * order + double(created.sec_since_epoch()) / double(T);
                }

                inline double calculate_hot(const share_type &score, const time_point_sec &created) const {
                    return calculate_score<10000000, 10000>(score, created);
                }

                inline double calculate_trending(const share_type &score, const time_point_sec &created) const {
                    return calculate_score<10000000, 480000>(score, created);
                }

                /** finds tags that have been added or  updated */
                void update_tags(const comment_object &c) const {
                    try{

                        auto hot = calculate_hot(c.net_rshares, c.created);
                        auto trending = calculate_trending(c.net_rshares, c.created);
                        auto language_ = filter_tags(c);
                        const auto &comment_idx = _db.get_index<language_index>().indices().get<by_comment>();

                        auto itr=comment_idx.find(c.id);

                        if (itr == comment_idx.end()) {
                            create_tag(language_, c, hot, trending);
                        } else {
                            update_tag( *itr, c, hot, trending);
                        }

                        if (c.parent_author.size()) {
                            update_tags(_db.get_comment(c.parent_author, c.parent_permlink));
                        }

                    } FC_CAPTURE_LOG_AND_RETHROW((c))

                }

                const peer_stats_object &get_or_create_peer_stats(account_id_type voter, account_id_type peer) const {
                    const auto &peeridx = _db.get_index<peer_stats_index>().indices().get<by_voter_peer>();
                    auto itr = peeridx.find(boost::make_tuple(voter, peer));
                    if (itr == peeridx.end()) {
                        return _db.create<peer_stats_object>([&](peer_stats_object &obj) {
                            obj.voter = voter;
                            obj.peer = peer;
                        });
                    }
                    return *itr;
                }

                void update_indirect_vote(account_id_type a, account_id_type b, int positive) const {
                    if (a == b) {
                        return;
                    }
                    const auto &ab = get_or_create_peer_stats(a, b);
                    const auto &ba = get_or_create_peer_stats(b, a);
                    _db.modify(ab, [&](peer_stats_object &o) {
                        o.indirect_positive_votes += positive;
                        o.indirect_votes++;
                        o.update_rank();
                    });
                    _db.modify(ba, [&](peer_stats_object &o) {
                        o.indirect_positive_votes += positive;
                        o.indirect_votes++;
                        o.update_rank();
                    });
                }

                void
                update_peer_stats(const account_object &voter, const account_object &author, const comment_object &c, int vote) const {
                    if (voter.id == author.id) {
                        return;
                    } /// ignore votes for yourself
                    if (c.parent_author.size()) {
                        return;
                    } /// only count top level posts

                    const auto &stat = get_or_create_peer_stats(voter.id, author.id);
                    _db.modify(stat, [&](peer_stats_object &obj) {
                        obj.direct_votes++;
                        obj.direct_positive_votes += vote > 0;
                        obj.update_rank();
                    });

                    const auto &voteidx = _db.get_index<comment_vote_index>().indices().get<by_comment_voter>();
                    auto itr = voteidx.lower_bound(boost::make_tuple(comment_id_type(c.id), account_id_type()));
                    while (itr != voteidx.end() && itr->comment == c.id) {
                        update_indirect_vote(voter.id, itr->voter, (itr->vote_percent > 0) == (vote > 0));
                        ++itr;
                    }
                }

                void operator()(const comment_operation &op) const {
                    update_tags(_db.get_comment(op.author, op.permlink));
                }

                void operator()(const transfer_operation &op) const {
                    if (op.to == STEEMIT_NULL_ACCOUNT &&
                        op.amount.symbol == SBD_SYMBOL) {
                        vector<string> part;
                        part.reserve(4);
                        auto path = op.memo;
                        boost::split(part, path, boost::is_any_of("/"));
                        if (part[0].size() && part[0][0] == '@') {
                            auto acnt = part[0].substr(1);
                            auto perm = part[1];

                            auto c = _db.find_comment(acnt, perm);
                            if (c && c->parent_author.size() == 0) {
                                const auto &comment_idx = _db.get_index<language_index>().indices().get<by_comment>();
                                auto citr = comment_idx.lower_bound(c->id);
                                while (citr != comment_idx.end() &&
                                       citr->comment == c->id) {
                                    _db.modify(*citr, [&](language_object &t) {
                                        if (t.mode == first_payout) {
                                            t.promoted_balance += op.amount.amount;
                                        }
                                    });
                                    ++citr;
                                }
                            } else {
                                ilog("unable to find body");
                            }
                        }
                    }
                }

                void operator()(const vote_operation &op) const {
                    update_tags(_db.get_comment(op.author, op.permlink));
                    /*
                    update_peer_stats( _db.get_account(op.voter),
                                       _db.get_account(op.author),
                                       _db.get_comment(op.author, op.permlink),
                                       op.weight );
                                       */
                }

                void operator()(const delete_comment_operation &op) const {
                    const auto &idx = _db.get_index<language_index>().indices().get<by_author_comment>();

                    const auto &auth = _db.get_account(op.author);
                    auto itr = idx.lower_bound(boost::make_tuple(auth.id));
                    while (itr != idx.end() && itr->author == auth.id) {
                        const auto &tobj = *itr;
                        const auto *obj = _db.find<comment_object>(itr->comment);
                        ++itr;
                        if (!obj) {
                            _db.remove(tobj);
                            languages_plugin_impl.self().cache_languages.erase(to_string(obj->languages));
                        }
                    }
                }

                void operator()(const comment_reward_operation &op) const {
                    const auto &c = _db.get_comment(op.author, op.permlink);
                    update_tags(c);
                }

                void operator()(const comment_payout_update_operation &op) const {
                    const auto &c = _db.get_comment(op.author, op.permlink);
                    update_tags(c);
                }

                template<typename Op>
                void operator()(Op &&) const {
                } /// ignore all other ops
            };


            void languages_plugin_impl::on_operation(const operation_notification &note) {
                try {
                    /// plugins shouldn't ever throw
                    note.op.visit(operation_visitor(self()));
                }
                catch (const fc::exception &e) {
                    edump((e.to_detail_string()));
                }
                catch (...) {
                    elog("unhandled exception");
                }
            }

        } /// end detail namespace

        languages_plugin::languages_plugin(application *app)
                : plugin(app), my(new detail::languages_plugin_impl(*this)) {
            chain::database &db = database();
            add_plugin_index<language_index>(db);
            add_plugin_index<language_stats_index>(db);
            add_plugin_index<peer_stats_index>(db);
            add_plugin_index<author_language_stats_index>(db);
        }

        languages_plugin::~languages_plugin() {
        }

        void languages_plugin::plugin_set_program_options(
                boost::program_options::options_description &cli,
                boost::program_options::options_description &cfg
        ) {
        }

        void languages_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
            ilog("Intializing languages plugin");
            database().post_apply_operation.connect(
                    [&](const operation_notification &note) { my->on_operation(note); });

            app().register_api_factory<language_api>("language_api");
        }


        void languages_plugin::plugin_startup() {
            database().with_read_lock(
                    [&]() {
                        const auto &index = my->database().get_index<language_index>().indices().get<languages::by_comment>();
                        auto itr = index.begin();
                        for (; itr != index.end(); ++itr) {
                            my->self().cache_languages.emplace(itr->language);
                        }
                    }
            );
            ilog("startup languages plugin");
        }

    }
} /// steemit::tags

STEEMIT_DEFINE_PLUGIN(languages, steemit::languages::languages_plugin)