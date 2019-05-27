#pragma once

#include <golos/plugins/operation_dump/operation_dump_plugin.hpp>
#include <golos/protocol/operations.hpp>
#include <golos/chain/comment_object.hpp>
#include <golos/plugins/follow/follow_operations.hpp>
#include <golos/plugins/tags/tag_visitor.hpp>
#include <queue>

namespace golos { namespace plugins { namespace operation_dump {

using namespace golos::plugins::follow;

#define COMMENT_ID(OP) std::string(OP.author) + "/" + OP.permlink

#define TAGS_NUMBER 15
#define TAG_MAX_LENGTH 512

class operation_dump_visitor {
public:
    using result_type = void;

    operation_dump_plugin& _plugin;

    const signed_block& _block;
    uint16_t& _op_in_block;

    database& _db;

    operation_dump_visitor(operation_dump_plugin& plugin, const signed_block& block, uint16_t& op_in_block, database& db)
            : _plugin(plugin), _block(block), _op_in_block(op_in_block), _db(db) {
    }

    void id_hash_pack(dump_buffer& b, const std::string& id) {
        fc::raw::pack(b, fc::hash64(id.c_str(), id.length()));
    }

    dump_buffer& write_op_header(const std::string& file_name, const std::string& op_related_id = "") {
        auto& b = _plugin.buffers[file_name];
        b.write(operation_number(_block.block_num(), _op_in_block));
        if (!op_related_id.empty()) {
            id_hash_pack(b, op_related_id);
        }
        return b;
    }

    template<typename T>
    auto operator()(const T&) -> result_type {
    }

    auto operator()(const transfer_operation& op) -> result_type {
        auto& b = write_op_header("transfers");

        fc::raw::pack(b, op);
        fc::raw::pack(b, _block.timestamp);
    }

    auto operator()(const comment_operation& op) -> result_type {
        auto& b = write_op_header("comments", COMMENT_ID(op));

        fc::raw::pack(b, op.parent_author);
        fc::raw::pack(b, op.parent_permlink);
        fc::raw::pack(b, op.author);
        fc::raw::pack(b, op.permlink);
        fc::raw::pack(b, op.title);
        fc::raw::pack(b, op.body);
        fc::raw::pack(b, golos::plugins::tags::get_metadata(op.json_metadata, TAGS_NUMBER, TAG_MAX_LENGTH));
        fc::raw::pack(b, _block.timestamp);
    }

    auto operator()(const delete_comment_operation& op) -> result_type {
        if (_db.find_comment(op.author, op.permlink)) {
            return;
        }

        write_op_header("delete_comments", COMMENT_ID(op));
    }

    auto operator()(const comment_benefactor_reward_operation& op) -> result_type {
        auto& b = write_op_header("benefactor_rewards", COMMENT_ID(op));

        fc::raw::pack(b, op);
    }

    auto operator()(const author_reward_operation& op) -> result_type {
        auto& b = write_op_header("author_rewards", COMMENT_ID(op));

        fc::raw::pack(b, op);
    }

    auto operator()(const curation_reward_operation& op) -> result_type {
        auto& b = write_op_header("curation_rewards", std::string(op.comment_author) + "/" + op.comment_permlink);

        fc::raw::pack(b, op);
    }

    auto operator()(const auction_window_reward_operation& op) -> result_type {
        auto& b = write_op_header("auction_window_rewards", std::string(op.comment_author) + "/" + op.comment_permlink);

        fc::raw::pack(b, op);
    }

    auto operator()(const total_comment_reward_operation& op) -> result_type {
        auto& b = write_op_header("total_comment_rewards", COMMENT_ID(op));

        fc::raw::pack(b, op);
    }

    auto operator()(const vote_operation& op) -> result_type {
        auto& b = write_op_header("votes", COMMENT_ID(op));

        fc::raw::pack(b, op);

        auto& block_rshares = _plugin.vote_rshares[_block.block_num()];
        fc::raw::pack(b, block_rshares.front());
        block_rshares.pop();

        fc::raw::pack(b, _block.timestamp);
    }

    // Not logs if operation failed in plugin, but logs if plugin not exists
    auto operator()(const custom_json_operation& op) -> result_type {
        if (op.id != "follow") { // follows, reblogs, delete_reblogs
            return;
        }

        std::vector<follow_plugin_operation> fpops;

        auto v = fc::json::from_string(op.json);
        try {
            if (v.is_array() && v.size() > 0 && v.get_array()[0].is_array()) {
                fc::from_variant(v, fpops);
            } else {
                fpops.emplace_back();
                fc::from_variant(v, fpops[0]);
            }
        } catch (...) {
            // Normal cases failed, try this strange case from follow-plugin
            try {
                auto fop = v.as<follow_operation>();
                fpops.emplace_back(fop);
            } catch (...) {
            }
        }

        for (const follow_plugin_operation& fpop : fpops) {
            fpop.visit(*this);
        }
    }

    auto operator()(const follow_operation& op) -> result_type {
        auto& b = write_op_header("follows", std::string(op.follower) + "/" + op.following);

        fc::raw::pack(b, op.follower);
        fc::raw::pack(b, op.following);

        uint16_t what = 0;
        for (const auto& target : op.what) {
            if (target == "blog") {
                what |= 1 << blog;
            } else if (target == "ignore") {
                what |= 1 << ignore;
            }
        }
        fc::raw::pack(b, what);
    }

    auto operator()(const reblog_operation& op) -> result_type {
        auto& b = write_op_header("reblogs", COMMENT_ID(op));

        fc::raw::pack(b, op.account);
        fc::raw::pack(b, op.author);
        fc::raw::pack(b, op.permlink);
        fc::raw::pack(b, op.title); // Usually empty, but no problems to dump
        fc::raw::pack(b, op.body);
        //fc::raw::pack(b, op.json_metadata); // Usually it is: {"app": "golos.io/0.1", "format": "text"} - not seems to be need
        fc::raw::pack(b, _block.timestamp);
    }

    auto operator()(const delete_reblog_operation& op) -> result_type {
        auto& b = write_op_header("delete_reblogs", COMMENT_ID(op));

        fc::raw::pack(b, op.account);
    }

    auto operator()(const account_create_operation& op) -> result_type {
        auto& b = write_op_header("account_metas");

        fc::raw::pack(b, op.new_account_name);
        fc::raw::pack(b, op.json_metadata);
    }

    auto operator()(const account_create_with_delegation_operation& op) -> result_type {
        auto& b = write_op_header("account_metas");

        fc::raw::pack(b, op.new_account_name);
        fc::raw::pack(b, op.json_metadata);
    }

    auto operator()(const account_update_operation& op) -> result_type {
        if (op.json_metadata.size() == 0) {
            return;
        }

        auto& b = write_op_header("account_metas");

        fc::raw::pack(b, op.account);
        fc::raw::pack(b, op.json_metadata);
    }

    auto operator()(const account_metadata_operation& op) -> result_type {
        auto& b = write_op_header("account_metas");

        fc::raw::pack(b, op.account);
        fc::raw::pack(b, op.json_metadata);
    }
};

} } } // golos::plugins::operation_dump
