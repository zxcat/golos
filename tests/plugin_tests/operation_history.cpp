#include <string>
#include <cstdint>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "database_fixture.hpp"
#include "comment_reward.hpp"

using golos::chain::add_operations_database_fixture;
using golos::plugins::operation_history::applied_operation;
using golos::plugins::json_rpc::msg_pack;

static const std::string OPERATIONS = "account_create_operation,delete_comment_operation,vote,comment";

struct operation_visitor {
    using result_type = std::string;
    template<class T>
    std::string operator()(const T&) const {
        return std::string(fc::get_typename<T>::name());
    }
};

void log_applied_options(const applied_operation &opts) {
    std::stringstream ss;
    ss << "[" << opts.block << "] ";
    ss << opts.trx_id.str() << " : "; /// golos::protocol::transaction_id_type
    operation_visitor ovisit;
    std::string op_name = opts.op.visit(ovisit);
    ss << "\"" << op_name << "\""; /// golos::protocol::operation
    ilog(ss.str());
}

struct operation_history_fixture : public add_operations_database_fixture {
    Operations check_operations() {
        uint32_t head_block_num = db->head_block_num();
        ilog("Check history operations, block num is " + std::to_string(head_block_num));

        Operations _founded_ops;
        operation_visitor ovisit;
        for (uint32_t i = 0; i <= head_block_num; ++i) {
            msg_pack mo;
            mo.args = std::vector<fc::variant>({fc::variant(i), fc::variant(false)});
            auto ops = oh_plugin->get_ops_in_block(mo);
            for (auto o : ops) {
                auto iter = _founded_ops.find(o.trx_id.str());
                if (iter == _founded_ops.end()) {
                    _founded_ops.insert(std::make_pair(o.trx_id.str(), o.op.visit(ovisit)));
                    log_applied_options(o);
                }
            }
        }
        return _founded_ops;
    }
};

BOOST_FIXTURE_TEST_SUITE(operation_history_plugin, operation_history_fixture)

BOOST_AUTO_TEST_CASE(operation_history_blocks) {
    const uint32_t HISTORY_BLOCKS = 2;
    BOOST_TEST_MESSAGE("Testing: operation_history_blocks");
    initialize({
            {"history-blocks",std::to_string(HISTORY_BLOCKS)},
            {"history-whitelist-ops",OPERATIONS}
    });

    auto _added_ops = add_operations();
    auto _founded_ops = check_operations();

    size_t _checked_ops_count = 0;
    for (auto it = _founded_ops.begin(); it != _founded_ops.end(); ++it) {
        auto iter = _added_ops.find(it->first);
        bool is_found = (iter != _added_ops.end());
        BOOST_CHECK(is_found);
        if (is_found) {
            BOOST_CHECK_EQUAL(iter->second, it->second);
            if (iter->second == it->second) {
                ++_checked_ops_count;
            }
        } else {
            BOOST_TEST_MESSAGE("Operation \"" + it->second + "\" by \"" + it->first + "\" is not found");
        }
    }
    BOOST_CHECK_EQUAL(_checked_ops_count, HISTORY_BLOCKS);
}

BOOST_AUTO_TEST_CASE(black_options_postfix) {
    BOOST_TEST_MESSAGE("Testing: black_options_postfix");
    initialize({{"history-blacklist-ops", OPERATIONS}});

    auto _added_ops = add_operations();
    auto _founded_ops = check_operations();

    size_t _chacked_ops_count = 0;
    for (const auto &co : _added_ops) {
        auto iter = _founded_ops.find(co.first);
        bool is_not_found = (iter == _founded_ops.end());
        BOOST_CHECK(is_not_found);
        if (is_not_found) {
            ++_chacked_ops_count;
        } else {
            BOOST_TEST_MESSAGE("Operation \"" + co.second + "\" by \"" + co.first + "\" is found");
        }
    }
    BOOST_CHECK_EQUAL(_chacked_ops_count, _added_ops.size());
}

BOOST_AUTO_TEST_CASE(white_options_postfix) {
    BOOST_TEST_MESSAGE("Testing: white_options_postfix");
    initialize({{"history-whitelist-ops", OPERATIONS}});

    auto _added_ops = add_operations();
    auto _founded_ops = check_operations();

    size_t _chacked_ops_count = 0;
    for (const auto &co : _added_ops) {
        auto iter = _founded_ops.find(co.first);
        bool is_found = (iter != _founded_ops.end());
        BOOST_CHECK(is_found);
        if (is_found) {
            BOOST_CHECK_EQUAL(iter->second, co.second);
            if (iter->second == co.second) {
                ++_chacked_ops_count;
            }
        } else {
            BOOST_TEST_MESSAGE("Operation \"" + co.second + "\" by \"" + co.first + "\" is not found");
        }
    }
    BOOST_CHECK_EQUAL(_chacked_ops_count, _added_ops.size());
}

BOOST_AUTO_TEST_CASE(short_operation_history_blocks) {
    BOOST_TEST_MESSAGE("Testing: short_operation_history_blocks");
    initialize({{"history-whitelist-ops","account_create_operation,delete_comment,comment"}});

    auto _added_ops = add_operations();
    auto _founded_ops = check_operations();

    size_t _chacked_ops_count = 0;
    for (auto it = _founded_ops.begin(); it != _founded_ops.end(); ++it) {
        auto iter = _added_ops.find(it->first);
        bool is_found = (iter != _added_ops.end());
        if (is_found) {
            BOOST_TEST_MESSAGE("Found operation \"" + it->second + "\" by \"" + it->first + "\"");
            BOOST_CHECK_EQUAL(iter->second, it->second);
            if (iter->second == it->second) {
                ++_chacked_ops_count;
            }
        }
    }
    BOOST_CHECK_EQUAL(_chacked_ops_count, 3);
}

BOOST_AUTO_TEST_SUITE_END()
