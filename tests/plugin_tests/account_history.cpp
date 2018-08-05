#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include "database_fixture.hpp"


using namespace golos::chain;
using golos::plugins::json_rpc::msg_pack;
using golos::plugins::operation_history::applied_operation;
using namespace golos::plugins::account_history;


struct account_history_fixture : public add_operations_database_fixture {
    using checked_accounts_map = std::map<uint32_t, std::set<std::string>>; // pair { [block], [[account names] operations]}

    checked_accounts_map check(const account_name_set& names) {
        checked_accounts_map _found_accs;
        for (auto n : names) {
            msg_pack mp;
            mp.args = std::vector<fc::variant>({fc::variant(n), fc::variant(100), fc::variant(100)});
            auto accs = ah_plugin->get_account_history(mp);
            for (auto a : accs) {
                auto it = _found_accs.find(a.second.block);
                if (it == _found_accs.end()) {
                    std::set<std::string> set;
                    set.insert(n);
                    _found_accs.insert(std::make_pair(a.second.block, set));
                } else {
                    it->second.insert(n);
                }
            }
        }
        return _found_accs;
    }
};


BOOST_FIXTURE_TEST_SUITE(account_history_plugin, account_history_fixture)

BOOST_AUTO_TEST_CASE(account_history_blocks) {
    BOOST_TEST_MESSAGE("Testing: account_history_blocks");
    const uint32_t HISTORY_BLOCKS = 3;
    initialize({{"history-blocks", std::to_string(HISTORY_BLOCKS)}});
    add_operations();

    account_name_set names = {"alice", "bob", "sam", "dave"};
    auto _found_accs = check(names);

    std::set<uint32_t> blocks;
    for (auto a : _found_accs) {
        for (auto n : a.second) {
            ilog("block:" + std::to_string(a.first) + ", \"" + n + "\"");
            auto iter = names.find(n);
            bool is_found = (iter != names.end());
            BOOST_CHECK(is_found);
            if (is_found) {
                blocks.insert(a.first);
            } else {
                BOOST_TEST_MESSAGE("Account [" + std::to_string(a.first) + "]: \"" + n + "\" is not found");
            }
        }
    }
    BOOST_CHECK_EQUAL(blocks.size(), HISTORY_BLOCKS);
}


///////////////////////////////////////////////////////////////
// filtering
///////////////////////////////////////////////////////////////
struct op_name_visitor {
    using result_type = std::string;
    template<class T>
    std::string operator()(const T&) const {
        auto n = std::string(fc::get_typename<T>::name());
        auto prefix_len = sizeof("golos::protocol::") - 1;
        return n.substr(prefix_len, n.size() - prefix_len + 1 - sizeof("_operation"));
    }
};

struct accounts_filter_fixture : public add_operations_database_fixture {
    accounts_filter_fixture() {}
    ~accounts_filter_fixture() {}

    using op_names = fc::flat_set<std::string>;

    void check_ops(const history_operations& ops, const std::string& expected) {
        op_name_visitor ovisit;
        std::vector<std::string> chk;
        if (!expected.empty()) {
            boost::split(chk, expected, boost::is_any_of("|"));
        }
        BOOST_CHECK_EQUAL(chk.size(), ops.size());
        int i = 0;
        for (const auto& o : ops) {
            const auto got_op = o.second.op.visit(ovisit);
            BOOST_CHECK_EQUAL(got_op, i < chk.size() ? chk[i++] : "OVERFLOW");
        }
    }

};

BOOST_FIXTURE_TEST_SUITE(account_history_filter, accounts_filter_fixture)

#define INITIAL_OPS "account_create|transfer|transfer_to_vesting"

BOOST_AUTO_TEST_CASE(get_account_history) {
    initialize();

    auto get_history = [this](const std::string& acc, const int from = -100, const int limit = -1) {
        msg_pack mp;
        mp.args = std::vector<fc::variant>({fc::variant(acc)});
        if (from != -100) {
            mp.args->push_back(fc::variant(from));
            if (limit != -1)
                mp.args->push_back(fc::variant(limit));
        }
        auto ops = ah_plugin->get_account_history(mp);
        return ops;
    };

    BOOST_TEST_MESSAGE("Testing: get_account_history");

    // NOTE: this test depends on operations generated with add_operations()
    add_operations();

    BOOST_TEST_MESSAGE("--- Test non-filtered history");
    auto alice_all = get_history("alice", -1, 10);
    check_ops(alice_all, INITIAL_OPS "|vote");

    auto bob_all = get_history("bob", -1, 10);
    check_ops(bob_all, INITIAL_OPS "|comment|vote|vote|vote|delete_comment");

    auto sam_all = get_history("sam", -1, 10);
    check_ops(sam_all, INITIAL_OPS "|vote");

    auto dave_all = get_history("dave", -1, 10);
    check_ops(dave_all, "account_create");

    auto cf_all = get_history("cyberfounder", -1, 10);
    check_ops(cf_all, "account_create|account_create|account_create|transfer|transfer|transfer|"
        "producer_reward|producer_reward|producer_reward|account_create|producer_reward");
}

BOOST_AUTO_TEST_CASE(filter_account_history_simple) {
   initialize();

    auto get_history = [this](const account_name_type& acc, account_history_query& q) {
        msg_pack mp;
        q.account = acc;
        mp.args = std::vector<fc::variant>({fc::variant(q)});
        auto ops = ah_plugin->filter_account_history(mp);
        return ops;
    };

    BOOST_TEST_MESSAGE("Testing: filter_account_history_simple");
    auto q = account_history_query();
    q.from = -1;
    q.limit = 10;

    BOOST_TEST_MESSAGE("--- generate ops");
    GOLOS_CHECK_NO_THROW(
        add_operations();
    );

    BOOST_TEST_MESSAGE("--- Test non-filtered history");
    auto alice_all = get_history("alice", q);
    check_ops(alice_all, INITIAL_OPS "|vote");

    auto bob_all = get_history("bob", q);
    check_ops(bob_all, INITIAL_OPS "|comment|vote|vote|vote|delete_comment");

    auto sam_all = get_history("sam", q);
    check_ops(sam_all, INITIAL_OPS "|vote");

    auto dave_all = get_history("dave", q);
    check_ops(dave_all, "account_create");

    auto cf_all = get_history("cyberfounder", q);
    check_ops(cf_all, "account_create|account_create|account_create|transfer|transfer|transfer|"
        "producer_reward|producer_reward|producer_reward|account_create|producer_reward");
}

BOOST_AUTO_TEST_CASE(filter_account_history) {
    initialize();

    auto get_history = [this](const account_name_type& acc, account_history_query& q) {
        msg_pack mp;
        q.account = acc;
        mp.args = std::vector<fc::variant>({fc::variant(q)});
        auto ops = ah_plugin->filter_account_history(mp);
        return ops;
    };

    BOOST_TEST_MESSAGE("Testing: filter_account_history");
    auto q = account_history_query();
    q.from = -1;
    q.limit = 10;

    BOOST_TEST_MESSAGE("--- generate ops");
    GOLOS_CHECK_NO_THROW(
        // add_operations(true);   /* generate with delays*/
        add_operations();
    );

    BOOST_TEST_MESSAGE("--- Test 'sender' direction");
    q.direction = operation_direction::sender;
    auto bob_sender = get_history("bob", q);
    check_ops(bob_sender, "transfer_to_vesting|comment|vote|delete_comment");

    BOOST_TEST_MESSAGE("--- Test 'receiver' direction");
    q.direction = operation_direction::receiver;
    auto dave_receiver = get_history("dave", q);
    check_ops(dave_receiver, "account_create");

    BOOST_TEST_MESSAGE("--- Test 'dual' direction");
    q.direction = operation_direction::dual;
    auto bob_dual = get_history("bob", q);
    check_ops(bob_dual, "transfer_to_vesting|vote|delete_comment");

    BOOST_TEST_MESSAGE("--- Test virtual only select");
    q.direction = operation_direction::any;
    q.select_ops = op_names({"VIRTUAL"});
    auto cf_vops = get_history("cyberfounder", q);
    check_ops(cf_vops, "producer_reward|producer_reward|producer_reward|producer_reward|producer_reward|producer_reward");

    BOOST_TEST_MESSAGE("--- Test virtual select with filtered op");
    q.filter_ops = op_names({"producer_reward_operation"});
    auto cf_vops2 = get_history("cyberfounder", q);
    check_ops(cf_vops2, "");

    BOOST_TEST_MESSAGE("--- Test pagination");
    generate_blocks(3);
    q.select_ops = op_names({"REAL", "producer_reward"});
    q.filter_ops = op_names({"account_create", "transfer"});
    q.limit = 6;
    auto cf_ops = get_history("cyberfounder", q);
    check_ops(cf_ops, "producer_reward|producer_reward|producer_reward|producer_reward|producer_reward|producer_reward|producer_reward");
    for (const auto& i: cf_ops) {
        uint32_t page2 = i.first;
        q.from = page2;
        q.limit = std::min(page2, uint32_t(6));
        auto cf_ops2 = get_history("cyberfounder", q);
        check_ops(cf_ops2, "producer_reward|custom|producer_reward|transfer_to_vesting|producer_reward");
        break;
    }
}

BOOST_AUTO_TEST_SUITE_END() // filter

BOOST_AUTO_TEST_SUITE_END()
