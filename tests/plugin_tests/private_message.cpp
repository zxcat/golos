#include <boost/test/unit_test.hpp>

#include "database_fixture.hpp"
#include "helpers.hpp"

#include <golos/chain/account_object.hpp>

#include <fc/crypto/aes.hpp>

#include <golos/plugins/private_message/private_message_plugin.hpp>
#include <golos/plugins/private_message/private_message_operations.hpp>

using golos::plugins::json_rpc::msg_pack;
using golos::logic_exception;
using golos::missing_object;
using golos::object_already_exist;
using namespace golos::protocol;
using namespace golos::plugins::private_message;

struct private_message_fixture : public golos::chain::database_fixture {
    private_message_fixture() : golos::chain::database_fixture() {
        initialize<private_message_plugin>();
        pm_plugin = appbase::app().find_plugin<private_message_plugin>();
        open_database();
        startup();
    }

    private_message_plugin* pm_plugin = nullptr;
};

fc::variant_object make_private_message_id(const std::string& from, const std::string& to, const uint64_t nonce) {
    auto res = fc::mutable_variant_object()("from",from)("to",to)("nonce",nonce);
    return fc::variant_object(res);
}

#define _CHECK_EQUAL_MESSAGE_API_OBJECT(L, R) \
    BOOST_CHECK_EQUAL((L).from, (R).from); \
    BOOST_CHECK_EQUAL((L).to, (R).to); \
    BOOST_CHECK_EQUAL((L).nonce, (R).nonce); \
    BOOST_CHECK_EQUAL((L).from_memo_key, (R).from_memo_key); \
    BOOST_CHECK_EQUAL((L).to_memo_key, (R).to_memo_key); \
    BOOST_CHECK_EQUAL((L).create_time, (R).create_time); \
    BOOST_CHECK_EQUAL((L).receive_time, (R).receive_time); \
    BOOST_CHECK_EQUAL((L).checksum, (R).checksum); \
    BOOST_CHECK_EQUAL((L).read_time, (R).read_time); \
    BOOST_CHECK_EQUAL((L).encrypted_message.size(), (R).encrypted_message.size()); \
    BOOST_CHECK_EQUAL(std::equal( \
       (L).encrypted_message.begin(), (L).encrypted_message.end(), \
       (R).encrypted_message.begin(), (R).encrypted_message.end()), true);

BOOST_FIXTURE_TEST_SUITE(private_message_plugin, private_message_fixture)

    BOOST_AUTO_TEST_CASE(private_send_message) {
        BOOST_TEST_MESSAGE("Testing: private_message_operation");

        ACTORS((alice)(bob));

        BOOST_TEST_MESSAGE("--- Send message");

        auto alice_bob_nonce = fc::time_point::now().time_since_epoch().count();
        auto alice_bob_secret = alice_private_key.get_shared_secret(bob_private_key.get_public_key());
        std::string alice_bob_msg = "Hello, Bob! My name is Alice.";
        std::vector<char> alice_bob_msg_data(alice_bob_msg.begin(), alice_bob_msg.end());

        auto bob_alice_nonce = alice_bob_nonce + 10;
        auto bob_alice_secret = bob_private_key.get_shared_secret(alice_private_key.get_public_key());
        std::string bob_alice_msg = "Hello, Alice!";
        std::vector<char> bob_alice_msg_data(bob_alice_msg.begin(), bob_alice_msg.end());

        BOOST_CHECK_EQUAL(alice_bob_secret, bob_alice_secret);

        fc::sha512::encoder enc;
        fc::raw::pack(enc, alice_bob_nonce);
        fc::raw::pack(enc, alice_bob_secret);
        auto alice_bob_encrypt_key = enc.result();
        uint32_t alice_bob_checksum = fc::sha256::hash(alice_bob_encrypt_key)._hash[0];

        private_message_operation mop;

        mop.from = "alice";
        mop.from_memo_key = alice_private_key.get_public_key();
        mop.to = "bob";
        mop.to_memo_key = bob_private_key.get_public_key();
        mop.nonce = alice_bob_nonce;
        mop.encrypted_message = fc::aes_encrypt(alice_bob_encrypt_key, alice_bob_msg_data);
        mop.checksum = alice_bob_checksum;

        private_message_plugin_operation pop = mop;

        custom_json_operation jop;
        jop.id = "private_message";
        jop.json = fc::json::to_string(pop);
        jop.required_posting_auths = {"alice"};

        signed_transaction trx;

        GOLOS_CHECK_NO_THROW(push_tx_with_ops(trx, alice_private_key, jop));

        enc.reset();
        fc::raw::pack(enc, bob_alice_nonce);
        fc::raw::pack(enc, bob_alice_secret);
        auto bob_alice_encrypt_key = enc.result();
        uint32_t bob_alice_checksum = fc::sha256::hash(bob_alice_encrypt_key)._hash[0];

        mop.from = "bob";
        mop.from_memo_key = bob_private_key.get_public_key();
        mop.to = "alice";
        mop.to_memo_key = alice_private_key.get_public_key();
        mop.nonce = bob_alice_nonce;
        mop.encrypted_message = fc::aes_encrypt(bob_alice_encrypt_key, bob_alice_msg_data);
        mop.checksum = bob_alice_checksum;

        pop = mop;
        jop.json = fc::json::to_string(pop);
        jop.required_posting_auths = {"bob"};

        GOLOS_CHECK_NO_THROW(push_tx_with_ops(trx, bob_private_key, jop));

        BOOST_TEST_MESSAGE("--- Get message ");

        msg_pack mp;
        mp.args = std::vector<fc::variant>({fc::variant("bob"), fc::variant(inbox_query())});
        auto bob_inbox = pm_plugin->get_inbox(mp);

        mp.args = std::vector<fc::variant>({fc::variant("bob"), fc::variant(outbox_query())});
        auto bob_outbox = pm_plugin->get_outbox(mp);

        mp.args = std::vector<fc::variant>({fc::variant("alice"), fc::variant(inbox_query())});
        auto alice_inbox = pm_plugin->get_inbox(mp);

        mp.args = std::vector<fc::variant>({fc::variant("alice"), fc::variant(outbox_query())});
        auto alice_outbox = pm_plugin->get_outbox(mp);

        mp.args = std::vector<fc::variant>({fc::variant("alice"), fc::variant("bob"), fc::variant(thread_query())});
        auto alice_bob_thread = pm_plugin->get_thread(mp);

        mp.args = std::vector<fc::variant>({fc::variant("bob"), fc::variant("alice"), fc::variant(thread_query())});
        auto bob_alice_thread = pm_plugin->get_thread(mp);

        BOOST_CHECK_EQUAL(bob_outbox.size(), 1);
        BOOST_CHECK_EQUAL(bob_inbox.size(), 1);
        BOOST_CHECK_EQUAL(alice_inbox.size(), 1);
        BOOST_CHECK_EQUAL(alice_outbox.size(), 1);
        BOOST_CHECK_EQUAL(alice_bob_thread.size(), 2);
        BOOST_CHECK_EQUAL(bob_alice_thread.size(), 2);

        _CHECK_EQUAL_MESSAGE_API_OBJECT(bob_outbox[0], alice_inbox[0]);
        _CHECK_EQUAL_MESSAGE_API_OBJECT(bob_outbox[0], alice_bob_thread[0]);
        _CHECK_EQUAL_MESSAGE_API_OBJECT(bob_outbox[0], bob_alice_thread[0]);
        BOOST_CHECK_EQUAL(bob_outbox[0].nonce, bob_alice_nonce);
        BOOST_CHECK_EQUAL(bob_outbox[0].checksum, bob_alice_checksum);

        _CHECK_EQUAL_MESSAGE_API_OBJECT(alice_outbox[0], bob_inbox[0]);
        _CHECK_EQUAL_MESSAGE_API_OBJECT(alice_outbox[0], alice_bob_thread[1]);
        _CHECK_EQUAL_MESSAGE_API_OBJECT(alice_outbox[0], bob_alice_thread[1]);
        BOOST_CHECK_EQUAL(alice_outbox[0].nonce, alice_bob_nonce);
        BOOST_CHECK_EQUAL(alice_outbox[0].checksum, alice_bob_checksum);

        BOOST_TEST_MESSAGE("--- Decrypt message");

        enc.reset();
        fc::raw::pack(enc, alice_inbox[0].nonce);
        fc::raw::pack(enc, alice_private_key.get_shared_secret(alice_inbox[0].from_memo_key));
        auto alice_bob_decrypt_key = enc.result();
        uint32_t alice_bob_decrypt_checksum = fc::sha256::hash(alice_bob_decrypt_key)._hash[0];

        BOOST_CHECK_EQUAL(alice_bob_decrypt_checksum, alice_inbox[0].checksum);

        auto alice_bob_decrypt_msg_data = fc::aes_decrypt(alice_bob_decrypt_key, alice_inbox[0].encrypted_message);

        BOOST_CHECK_EQUAL(alice_bob_decrypt_msg_data.size(), bob_alice_msg_data.size());
        BOOST_CHECK_EQUAL(std::equal(
            alice_bob_decrypt_msg_data.begin(), alice_bob_decrypt_msg_data.end(),
            bob_alice_msg_data.begin(), bob_alice_msg_data.end()), true);

        enc.reset();
        fc::raw::pack(enc, bob_inbox[0].nonce);
        fc::raw::pack(enc, bob_private_key.get_shared_secret(bob_inbox[0].from_memo_key));
        auto bob_alice_decrypt_key = enc.result();
        uint32_t bob_alice_decrypt_checksum = fc::sha256::hash(bob_alice_decrypt_key)._hash[0];

        BOOST_CHECK_EQUAL(bob_alice_decrypt_checksum, bob_inbox[0].checksum);

        auto bob_alice_decrypt_msg_data = fc::aes_decrypt(bob_alice_decrypt_key, bob_inbox[0].encrypted_message);

        BOOST_CHECK_EQUAL(bob_alice_decrypt_msg_data.size(), alice_bob_msg_data.size());
        BOOST_CHECK_EQUAL(std::equal(
            bob_alice_decrypt_msg_data.begin(), bob_alice_decrypt_msg_data.end(),
            alice_bob_msg_data.begin(), alice_bob_msg_data.end()), true);

        BOOST_TEST_MESSAGE("--- Edit message");

        std::string alice_bob_edited_msg = "Edited: Hello, Bob! My name is Alice.";
        std::vector<char> alice_bob_edited_msg_data(alice_bob_edited_msg.begin(), alice_bob_edited_msg.end());

        mop.from = "alice";
        mop.from_memo_key = alice_private_key.get_public_key();
        mop.to = "bob";
        mop.to_memo_key = bob_private_key.get_public_key();
        mop.nonce = alice_bob_nonce + 1000;
        mop.update = true;
        mop.encrypted_message = fc::aes_encrypt(alice_bob_encrypt_key, alice_bob_edited_msg_data);
        mop.checksum = alice_bob_checksum;

        pop = mop;

        jop.json = fc::json::to_string(pop);
        jop.required_posting_auths = {"alice"};

        GOLOS_CHECK_ERROR_PROPS(push_tx_with_ops(trx, alice_private_key, jop),
            CHECK_ERROR(tx_invalid_operation, 0,
                CHECK_ERROR(missing_object, "private_message",
                    make_private_message_id("alice", "bob", mop.nonce))));

        mop.nonce = alice_bob_nonce;
        mop.update = false;
        pop = mop;
        jop.json = fc::json::to_string(pop);

        GOLOS_CHECK_ERROR_PROPS(push_tx_with_ops(trx, alice_private_key, jop),
            CHECK_ERROR(tx_invalid_operation, 0,
                CHECK_ERROR(object_already_exist, "private_message",
                    make_private_message_id("alice", "bob", mop.nonce))));

        mop.update = true;
        pop = mop;
        jop.json = fc::json::to_string(pop);
        GOLOS_CHECK_NO_THROW(push_tx_with_ops(trx, alice_private_key, jop));

        mp.args = std::vector<fc::variant>({fc::variant("bob"), fc::variant(inbox_query())});
        bob_inbox = pm_plugin->get_inbox(mp);

        mp.args = std::vector<fc::variant>({fc::variant("alice"), fc::variant(outbox_query())});
        alice_outbox = pm_plugin->get_outbox(mp);

        BOOST_CHECK_EQUAL(bob_inbox.size(), 1);
        BOOST_CHECK_EQUAL(alice_outbox.size(), 1);
        _CHECK_EQUAL_MESSAGE_API_OBJECT(bob_inbox[0], alice_outbox[0]);

        auto bob_alice_edited_decrypt_msg_data = fc::aes_decrypt(bob_alice_decrypt_key, bob_inbox[0].encrypted_message);

        BOOST_CHECK_EQUAL(bob_alice_edited_decrypt_msg_data.size(), alice_bob_edited_msg_data.size());
        BOOST_CHECK_EQUAL(std::equal(
            bob_alice_edited_decrypt_msg_data.begin(), bob_alice_edited_decrypt_msg_data.end(),
            alice_bob_edited_msg_data.begin(), alice_bob_edited_msg_data.end()), true);
    }

BOOST_AUTO_TEST_SUITE_END()
