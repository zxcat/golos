#include <boost/program_options/options_description.hpp>
#include <golos/plugins/social_network/social_network.hpp>
#include <golos/chain/index.hpp>
#include <golos/api/vote_state.hpp>
#include <golos/chain/steem_objects.hpp>

#include <golos/api/discussion_helper.hpp>
// These visitors creates additional tables, we don't really need them in LOW_MEM mode
#include <golos/plugins/tags/plugin.hpp>
#include <golos/chain/operation_notification.hpp>
#include <golos/protocol/types.hpp>
#include <golos/protocol/config.hpp>

#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>

#include <golos/protocol/config.hpp>

#define CHECK_ARG_SIZE(_S)                                 \
   FC_ASSERT(                                              \
       args.args->size() == _S,                            \
       "Expected #_S argument(s), was ${n}",               \
       ("n", args.args->size()) );

#define CHECK_ARG_MIN_SIZE(_S, _M)                         \
   FC_ASSERT(                                              \
       args.args->size() >= _S && args.args->size() <= _M, \
       "Expected #_S (maximum #_M) argument(s), was ${n}", \
       ("n", args.args->size()) );

#define GET_OPTIONAL_ARG(_I, _T, _D)   \
   (args.args->size() > _I) ?          \
   (args.args->at(_I).as<_T>()) :      \
   static_cast<_T>(_D)

#ifndef DEFAULT_VOTE_LIMIT
#  define DEFAULT_VOTE_LIMIT 10000
#endif


// Depth of comment_content information storage history.
struct content_depth_params {
    content_depth_params() {
    }

    inline bool miss_content() const {
        return has_comment_title_depth && !comment_title_depth &&
               has_comment_body_depth && !comment_body_depth &&
               has_comment_json_metadata_depth && !comment_json_metadata_depth;
    }

    inline bool need_clear() const {
        return has_comment_title_depth || has_comment_body_depth || has_comment_json_metadata_depth;
    }

    inline bool should_delete_whole_content_object(const uint32_t & delta) const {
        return comment_title_depth > delta && comment_body_depth > delta && comment_json_metadata_depth > delta;
    }

    inline bool should_delete_part_of_content_object(const uint32_t & delta) const {
        return comment_title_depth > delta || comment_body_depth > delta || comment_json_metadata_depth > delta;
    }


    uint32_t comment_title_depth;
    uint32_t comment_body_depth;
    uint32_t comment_json_metadata_depth;

    bool has_comment_title_depth = false;
    bool has_comment_body_depth = false;
    bool has_comment_json_metadata_depth = false;

    bool set_null_after_update = false;
};

namespace golos { namespace plugins { namespace social_network {
    using golos::plugins::tags::fill_promoted;
    using golos::api::discussion_helper;

    using boost::locale::conv::utf_to_utf;

    struct social_network::impl final {
        impl(): database_(appbase::app().get_plugin<chain::plugin>().db()) {
            helper = std::make_unique<discussion_helper>(database_, follow::fill_account_reputation, fill_promoted, get_comment_content_callback);
        }

        ~impl() = default;

        golos::chain::database& database() {
            return database_;
        }

        golos::chain::database& database() const {
            return database_;
        }

        void select_active_votes (
            std::vector<vote_state>& result, uint32_t& total_count,
            const std::string& author, const std::string& permlink, uint32_t limit
        ) const ;

        void select_content_replies(
            std::vector<discussion>& result, std::string author, std::string permlink, uint32_t limit
        ) const;

        std::vector<discussion> get_content_replies(
            const std::string& author, const std::string& permlink, uint32_t vote_limit
        ) const;

        std::vector<discussion> get_all_content_replies(
            const std::string& author, const std::string& permlink, uint32_t vote_limit
        ) const;

        std::vector<discussion> get_replies_by_last_update(
            account_name_type start_parent_author, std::string start_permlink,
            uint32_t limit, uint32_t vote_limit
        ) const;

        discussion get_content(std::string author, std::string permlink, uint32_t limit) const;

        discussion get_discussion(const comment_object& c, uint32_t vote_limit) const ;
 
        void set_depth_parameters(const content_depth_params &params);

        // Looks for a comment_operation, fills the comment_content state objects.
        void post_operation(const operation_notification &o);

        void on_block(const signed_block &b);

        comment_api_object create_comment_api_object(const comment_object & o) const ;

        const comment_content_object &get_comment_content(const comment_id_type &comment) const ;

        const comment_content_object *find_comment_content(const comment_id_type &comment) const ;


    private:
        golos::chain::database& database_;
        std::unique_ptr<discussion_helper> helper;
        content_depth_params depth_parameters;
    };


    const comment_content_object &social_network::impl::get_comment_content(const comment_id_type &comment) const {
        try {
            return database().get<comment_content_object, by_comment>(comment);
        } FC_CAPTURE_AND_RETHROW((comment))
    }

    const comment_content_object &social_network::get_comment_content(const comment_id_type &comment) const {
        return pimpl->get_comment_content(comment);
    }


    const comment_content_object *social_network::impl::find_comment_content(const comment_id_type &comment) const {
        return database().find<comment_content_object, by_comment>(comment);
    }
     
    const comment_content_object *social_network::find_comment_content(const comment_id_type &comment) const {
        return pimpl->find_comment_content(comment);
    }

    discussion social_network::impl::get_discussion(const comment_object& c, uint32_t vote_limit) const {
        return helper->get_discussion(c, vote_limit);
    }

    void social_network::impl::select_active_votes(
        std::vector<vote_state>& result, uint32_t& total_count,
        const std::string& author, const std::string& permlink, uint32_t limit
    ) const {
        helper->select_active_votes(result, total_count, author, permlink, limit);
    }

    void social_network::impl::set_depth_parameters(const content_depth_params &params) {
        depth_parameters = params;
    }

    struct operation_visitor {
        using result_type = void;

        golos::chain::database &db;
        content_depth_params depth_parameters;

        operation_visitor(golos::chain::database &db, const content_depth_params &params) : db(db), depth_parameters(params) {
        }

        std::wstring utf8_to_wstring(const std::string &str) const {
            return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
        }

        std::string wstring_to_utf8(const std::wstring &str) const {
            return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
        }

        const comment_content_object &get_comment_content(const comment_id_type &comment) const {
            try {
                return db.get<comment_content_object, by_comment>(comment);
            } FC_CAPTURE_AND_RETHROW((comment))
        }

        const comment_content_object *find_comment_content(const comment_id_type &comment) const {
            return db.find<comment_content_object, by_comment>(comment);
        }

        template<class T>
        void operator()(const T& o) const {
        }

        void operator()(const delete_comment_operation& o) const {
            const auto &comment = db.get_comment(o.author, o.permlink);
            auto& content = get_comment_content(comment.id);
            db.remove(content);
        }

        void operator()(const golos::protocol::comment_operation& o) const {
            if (depth_parameters.miss_content()) {
                return;
            }

            const auto& comment = db.get_comment(o.author, o.permlink);

            if (comment.created != db.head_block_time()) {
                // Edit case
                db.modify(db.get< comment_content_object, by_comment >( comment.id ), [&]( comment_content_object& con ) {
                    if (o.title.size() && (!depth_parameters.has_comment_title_depth || depth_parameters.comment_title_depth > 0)) {
                        from_string(con.title, o.title);
                    }
                    if (o.json_metadata.size()) {
                        if ((!depth_parameters.has_comment_json_metadata_depth || depth_parameters.comment_json_metadata_depth > 0) &&
                            fc::is_utf8(o.json_metadata)
                        ) {
                            from_string(con.json_metadata, o.json_metadata );
                        }
                        else {
                            wlog("Comment ${a}/${p} contains invalid UTF-8 metadata", ("a", o.author)("p", o.permlink));
                        }
                    }
                    if (o.body.size() && (!depth_parameters.has_comment_body_depth || depth_parameters.comment_body_depth > 0)) {
                        try {
                            diff_match_patch<std::wstring> dmp;
                            auto patch = dmp.patch_fromText(utf8_to_wstring(o.body));
                            if (patch.size()) {
                                auto result = dmp.patch_apply(patch, utf8_to_wstring(to_string(con.body)));
                                auto patched_body = wstring_to_utf8(result.first);
                                if(!fc::is_utf8(patched_body)) {
                                    idump(("invalid utf8")(patched_body));
                                    from_string(con.body, fc::prune_invalid_utf8(patched_body));
                                }
                                else {
                                    from_string(con.body, patched_body);
                                }
                            }
                            else { // replace
                                from_string(con.body, o.body);
                            }
                        } catch ( ... ) {
                            from_string(con.body, o.body);
                        }
                    }
                    // Set depth null if needed (this parameter is given in config)
                    if (depth_parameters.set_null_after_update) {
                        con.block_number = db.head_block_num();
                    } 
                });

            }
            else {
                // Creation case

                const auto &new_comment = db.get_comment(o.author, o.permlink);
                comment_id_type id = new_comment.id;

                db.create<comment_content_object>([&](comment_content_object& con) {
                    con.comment = id;
                    if (!depth_parameters.has_comment_title_depth || depth_parameters.comment_title_depth > 0) {
                        from_string(con.title, o.title);
                    }
                    
                    if ((!depth_parameters.has_comment_body_depth || depth_parameters.comment_body_depth > 0) && o.body.size() < 1024*1024*128) {
                        from_string(con.body, o.body);
                    }

                    if ((!depth_parameters.has_comment_json_metadata_depth || depth_parameters.comment_json_metadata_depth > 0) &&
                        fc::is_utf8(o.json_metadata)
                    ) {
                        from_string(con.json_metadata, o.json_metadata);
                    }
                    else {
                        wlog("Comment ${a}/${p} contains invalid UTF-8 metadata",
                            ("a", o.author)("p", o.permlink));
                    }

                    con.block_number = db.head_block_num();
                });
            }
        }

        // Checking should we delete comment_content if needed depth has been expired
        void operator()(const comment_payout_update_operation& o) const { 
            if (!depth_parameters.need_clear()) {
                return;
            }

            const auto &comment = db.get_comment(o.author, o.permlink);
            auto& content = get_comment_content(comment.id);
            auto delta = db.head_block_num() - content.block_number;

            if (depth_parameters.should_delete_whole_content_object(delta)) {
                db.remove(content);
                return;
            }

            if (depth_parameters.should_delete_part_of_content_object(delta)) {
                db.modify(content, [&](comment_content_object& con) {
                    if (delta > depth_parameters.comment_title_depth) {
                        con.title.clear();
                    }

                    if (delta > depth_parameters.comment_body_depth) {
                        con.body.clear();
                    }

                    if (delta > depth_parameters.comment_json_metadata_depth) {
                        con.json_metadata.clear();
                    }
                });

                return; 
            }
        }

    };

    void social_network::impl::post_operation(const operation_notification &o) {
        auto& db = database();

        operation_visitor ovisit(db, depth_parameters);

        o.op.visit(ovisit);
    }
    void social_network::impl::on_block(const signed_block &b) {
        auto & db = database();

        const auto &idx = db.get_index<comment_content_index>().indices().get<by_block_number>();

        for (auto itr = idx.begin(); itr != idx.end(); ++itr) {
            auto & content = *itr;

            const auto &cidx = db.get_index<comment_index>().indices().get<by_id>();
        
            auto comment = cidx.find(content.comment);

            int64_t cash_window_sec = STEEMIT_CASHOUT_WINDOW_SECONDS;
            auto time_delta = db.head_block_time() - comment->created;
            auto delta = db.head_block_num() - content.block_number;

            if (time_delta > fc::microseconds(cash_window_sec) && depth_parameters.should_delete_part_of_content_object(delta)) {
                if (depth_parameters.should_delete_whole_content_object(delta)) {
                    db.remove(content);
                    continue;
                }
                db.modify(content, [&](comment_content_object& con) {
                    if (delta > depth_parameters.comment_title_depth) {
                        con.title.clear();
                    }

                    if (delta > depth_parameters.comment_body_depth) {
                        con.body.clear();
                    }

                    if (delta > depth_parameters.comment_json_metadata_depth) {
                        con.json_metadata.clear();
                    }
                });

            }
        }
    }


    void social_network::plugin_startup() {
        wlog("social_network plugin: plugin_startup()");
    }

    void social_network::plugin_shutdown() {
        wlog("social_network plugin: plugin_shutdown()");
    }

    const std::string& social_network::name() {
        static const std::string name = "social_network";
        return name;
    }

    social_network::social_network() = default;

    void social_network::set_program_options(
        boost::program_options::options_description& cfg,
        boost::program_options::options_description& config_file_options
    ) {
        cfg.add_options()
            ( // Depth of comment_content information storage history.
                "comment-title-depth", boost::program_options::value<uint32_t>(),
                "max count of storing records of comment.title"
            ) (
                "comment-body-depth", boost::program_options::value<uint32_t>(),
                "max count of storing records of comment.body"
            ) (
                "comment-json-metadata-depth", boost::program_options::value<uint32_t>(),
                "max count of storing records of comment.json_metadata"
            ) (
                "set-content-storing-depth-null-after-update", boost::program_options::value<bool>()->default_value(false),
                "max count of storing records of comment.json_metadata"
            );
    }

    void social_network::plugin_initialize(const boost::program_options::variables_map& options) {
        pimpl = std::make_unique<impl>();
        JSON_RPC_REGISTER_API(name());

        auto& db = pimpl->database();

        add_plugin_index<comment_content_index>(db);

        db.post_apply_operation.connect([&](const operation_notification &o) {
            pimpl->post_operation(o);
        });

        db.applied_block.connect([&](const signed_block &b) {
            pimpl->on_block(b);
        });

        content_depth_params params;

        if (options.count("comment-title-depth")) {
            params.comment_title_depth = options.at("comment-title-depth").as<uint32_t>();
            params.has_comment_title_depth = true;
        }

        if (options.count("comment-body-depth")) {
            params.comment_body_depth = options.at("comment-body-depth").as<uint32_t>();
            params.has_comment_body_depth = true;
        }

        if (options.count("comment-json-metadata-depth")) {
            params.comment_json_metadata_depth = options.at("comment-json-metadata-depth").as<uint32_t>();
            params.has_comment_json_metadata_depth = true;
        }

        if (options.count("set-content-storing-depth-null-after-update")) {
            params.set_null_after_update = options.at("set-content-storing-depth-null-after-update").as<bool>();
        }

        pimpl->set_depth_parameters(params);
    }

    social_network::~social_network() = default;

    comment_api_object social_network::impl::create_comment_api_object(const comment_object & o) const {
        return helper->create_comment_api_object(o);
    }

    comment_api_object social_network::create_comment_api_object(const comment_object & o) const {
        return pimpl->create_comment_api_object(o);
    }

    void social_network::impl::select_content_replies(
        std::vector<discussion>& result, std::string author, std::string permlink, uint32_t limit
    ) const {
        account_name_type acc_name = account_name_type(author);
        const auto& by_permlink_idx = database().get_index<comment_index>().indices().get<by_parent>();
        auto itr = by_permlink_idx.find(std::make_tuple(acc_name, permlink));
        while (
            itr != by_permlink_idx.end() &&
            itr->parent_author == author &&
            to_string(itr->parent_permlink) == permlink
        ) {
            result.emplace_back(get_discussion(*itr, limit));
            ++itr;
        }
    }

    std::vector<discussion> social_network::impl::get_content_replies(
        const std::string& author, const std::string& permlink, uint32_t vote_limit
    ) const {
        std::vector<discussion> result;
        select_content_replies(result, author, permlink, vote_limit);
        return result;
    }

    DEFINE_API(social_network, get_content_replies) {
        CHECK_ARG_MIN_SIZE(2, 3)
        auto author = args.args->at(0).as<string>();
        auto permlink = args.args->at(1).as<string>();
        auto vote_limit = GET_OPTIONAL_ARG(2, uint32_t, DEFAULT_VOTE_LIMIT);
        return pimpl->database().with_weak_read_lock([&]() {
            return pimpl->get_content_replies(author, permlink, vote_limit);
        });
    }

    std::vector<discussion> social_network::impl::get_all_content_replies(
        const std::string& author, const std::string& permlink, uint32_t vote_limit
    ) const {
        std::vector<discussion> result;
        select_content_replies(result, author, permlink, vote_limit);
        for (std::size_t i = 0; i < result.size(); ++i) {
            if (result[i].children > 0) {
                auto j = result.size();
                select_content_replies(result, result[i].author, result[i].permlink, vote_limit);
                for (; j < result.size(); ++j) {
                    result[i].replies.push_back(result[j].author + "/" + result[j].permlink);
                }
            }
        }
        return result;
    }

    DEFINE_API(social_network, get_all_content_replies) {
        CHECK_ARG_MIN_SIZE(2, 3)
        auto author = args.args->at(0).as<string>();
        auto permlink = args.args->at(1).as<string>();
        auto vote_limit = GET_OPTIONAL_ARG(2, uint32_t, DEFAULT_VOTE_LIMIT);
        return pimpl->database().with_weak_read_lock([&]() {
            return pimpl->get_all_content_replies(author, permlink, vote_limit);
        });
    }

    DEFINE_API(social_network, get_account_votes) {
        CHECK_ARG_MIN_SIZE(1, 3)
        account_name_type voter = args.args->at(0).as<account_name_type>();
        auto from = GET_OPTIONAL_ARG(1, uint32_t, 0);
        auto limit = GET_OPTIONAL_ARG(2, uint64_t, DEFAULT_VOTE_LIMIT);

        auto& db = pimpl->database();
        return db.with_weak_read_lock([&]() {
            std::vector<account_vote> result;

            const auto& voter_acnt = db.get_account(voter);
            const auto& idx = db.get_index<comment_vote_index>().indices().get<by_voter_comment>();

            account_object::id_type aid(voter_acnt.id);
            auto itr = idx.lower_bound(aid);
            auto end = idx.upper_bound(aid);

            limit += from;
            for (uint32_t i = 0; itr != end && i < limit; ++itr, ++i) {
                if (i < from) {
                    continue;
                }

                const auto& vo = db.get(itr->comment);
                account_vote avote;
                avote.authorperm = vo.author + "/" + to_string(vo.permlink);
                avote.weight = itr->weight;
                avote.rshares = itr->rshares;
                avote.percent = itr->vote_percent;
                avote.time = itr->last_update;
                result.emplace_back(avote);
            }
            return result;
        });
    }

    discussion social_network::impl::get_content(std::string author, std::string permlink, uint32_t limit) const {
        const auto& by_permlink_idx = database().get_index<comment_index>().indices().get<by_permlink>();
        auto itr = by_permlink_idx.find(std::make_tuple(author, permlink));
        if (itr != by_permlink_idx.end()) {
            return get_discussion(*itr, limit);
        }
        return helper->create_discussion(*itr);
    }

    DEFINE_API(social_network, get_content) {
        CHECK_ARG_MIN_SIZE(2, 3)
        auto author = args.args->at(0).as<account_name_type>();
        auto permlink = args.args->at(1).as<string>();
        auto vote_limit = GET_OPTIONAL_ARG(2, uint32_t, DEFAULT_VOTE_LIMIT);
        return pimpl->database().with_weak_read_lock([&]() {
            return pimpl->get_content(author, permlink, vote_limit);
        });
    }

    DEFINE_API(social_network, get_active_votes) {
        CHECK_ARG_MIN_SIZE(2, 3)
        auto author = args.args->at(0).as<string>();
        auto permlink = args.args->at(1).as<string>();
        auto limit = GET_OPTIONAL_ARG(2, uint32_t, DEFAULT_VOTE_LIMIT);
        return pimpl->database().with_weak_read_lock([&]() {
            std::vector<vote_state> result;
            uint32_t total_count;
            pimpl->select_active_votes(result, total_count, author, permlink, limit);
            return result;
        });
    }

    std::vector<discussion> social_network::impl::get_replies_by_last_update(
        account_name_type start_parent_author,
        std::string start_permlink,
        uint32_t limit,
        uint32_t vote_limit
    ) const {
        std::vector<discussion> result;
#ifndef IS_LOW_MEM
        auto& db = database();
        const auto& last_update_idx = db.get_index<comment_index>().indices().get<by_last_update>();
        auto itr = last_update_idx.begin();
        const account_name_type* parent_author = &start_parent_author;

        if (start_permlink.size()) {
            const auto& comment = db.get_comment(start_parent_author, start_permlink);
            itr = last_update_idx.iterator_to(comment);
            parent_author = &comment.parent_author;
        } else if (start_parent_author.size()) {
            itr = last_update_idx.lower_bound(start_parent_author);
        }

        result.reserve(limit);

        while (itr != last_update_idx.end() && result.size() < limit && itr->parent_author == *parent_author) {
            result.emplace_back(get_discussion(*itr, vote_limit));
            ++itr;
        }
#endif
        return result;
    }

    /**
     *  This method can be used to fetch replies to an account.
     *
     *  The first call should be (account_to_retrieve replies, "", limit)
     *  Subsequent calls should be (last_author, last_permlink, limit)
     */
    DEFINE_API(social_network, get_replies_by_last_update) {
        CHECK_ARG_MIN_SIZE(3, 4)
        auto start_parent_author = args.args->at(0).as<account_name_type>();
        auto start_permlink = args.args->at(1).as<string>();
        auto limit = args.args->at(2).as<uint32_t>();
        auto vote_limit = GET_OPTIONAL_ARG(3, uint32_t, DEFAULT_VOTE_LIMIT);
        FC_ASSERT(limit <= 100);
        return pimpl->database().with_weak_read_lock([&]() {
            return pimpl->get_replies_by_last_update(start_parent_author, start_permlink, limit, vote_limit);
        });
    }

    
    get_comment_content_res get_comment_content_callback(const golos::chain::database & db, const comment_object & o) {
        if (!db.has_index<comment_content_index>()) {
            return get_comment_content_res();
        }
        auto & content = db.get<comment_content_object, by_comment>(o.id);

        get_comment_content_res result;

        result.title = std::string(content.title.begin(), content.title.end());
        result.body = std::string(content.body.begin(), content.body.end());
        result.json_metadata = std::string(content.json_metadata.begin(), content.json_metadata.end());

        return result;
    }

} } } // golos::plugins::social_network
