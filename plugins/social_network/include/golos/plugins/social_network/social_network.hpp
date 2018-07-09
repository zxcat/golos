#pragma once

#include <appbase/application.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <golos/api/discussion.hpp>
#include <golos/plugins/follow/plugin.hpp>
#include <golos/api/account_vote.hpp>
#include <golos/api/vote_state.hpp>
#include <golos/api/discussion_helper.hpp>


namespace golos { namespace plugins { namespace social_network {
    using plugins::json_rpc::msg_pack;
    using golos::api::discussion;
    using golos::api::account_vote;
    using golos::api::vote_state;
    using golos::api::get_comment_content_res;
    using namespace golos::chain;

    DEFINE_API_ARGS(get_content,                msg_pack, discussion)
    DEFINE_API_ARGS(get_content_replies,        msg_pack, std::vector<discussion>)
    DEFINE_API_ARGS(get_all_content_replies,    msg_pack, std::vector<discussion>)
    DEFINE_API_ARGS(get_account_votes,          msg_pack, std::vector<account_vote>)
    DEFINE_API_ARGS(get_active_votes,           msg_pack, std::vector<vote_state>)
    DEFINE_API_ARGS(get_replies_by_last_update, msg_pack, std::vector<discussion>)

    class social_network final: public appbase::plugin<social_network> {
    public:
        APPBASE_PLUGIN_REQUIRES (
            (chain::plugin)
            (json_rpc::plugin)
        )

        DECLARE_API(
            (get_content)
            (get_content_replies)
            (get_all_content_replies)
            (get_account_votes)
            (get_active_votes)
            (get_replies_by_last_update)
        )

        social_network();
        ~social_network();

        void set_program_options(
            boost::program_options::options_description& cfg,
            boost::program_options::options_description& config_file_options
        ) override;

        static const std::string& name();

        void plugin_initialize(const boost::program_options::variables_map& options) override;

        void plugin_startup() override;
        void plugin_shutdown() override;

    private:
        struct impl;
        std::unique_ptr<impl> pimpl;
    };

    get_comment_content_res get_comment_content_callback(const golos::chain::database & db, const comment_object & o);

#ifndef SOCIAL_NETWORK_SPACE_ID
#define SOCIAL_NETWORK_SPACE_ID 23
#endif

    enum social_network_types {
        comment_content_object_type = (SOCIAL_NETWORK_SPACE_ID << 8) + 1
    };


    class comment_content_object
            : public object<comment_content_object_type, comment_content_object> {
    public:
        comment_content_object() = delete;

        template<typename Constructor, typename Allocator>
        comment_content_object(Constructor &&c, allocator <Allocator> a)
                :title(a), body(a), json_metadata(a) {
            c(*this);
        }

        id_type id;

        comment_id_type   comment;

        shared_string title;
        shared_string body;
        shared_string json_metadata;
    };


    using comment_content_id_type = object_id<comment_content_object>;

    struct by_comment;

    typedef multi_index_container<
          comment_content_object,
          indexed_by<
             ordered_unique<tag<by_id>, member<comment_content_object, comment_content_id_type, &comment_content_object::id>>,
             ordered_unique<tag<by_comment>, member<comment_content_object, comment_id_type, &comment_content_object::comment>>>,
        allocator<comment_content_object>
    > comment_content_index;

// Callback which is needed for correct work of discussion_helper
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

CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::social_network::comment_content_object, 
    golos::plugins::social_network::comment_content_index
)