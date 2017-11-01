#include <steemit/plugins/account_by_key/account_by_key_plugin.hpp>
#include <steemit/plugins/account_by_key/account_by_key_objects.hpp>
#include <steemit/chain/database.hpp>
#include <steemit/plugins/chain/chain_plugin.hpp>
#include <steemit/chain/objects/account_object.hpp>
#include <steemit/chain/operation_notification.hpp>

namespace steemit {
    namespace plugins {
        namespace account_by_key {
            using namespace steemit::chain;
            struct account_by_key_plugin::account_by_key_plugin_impl final {
            public:
                account_by_key_plugin_impl(account_by_key_plugin &plugin)
                        : database_(appbase::app().get_plugin<chain_interface::chain_plugin>().db()),
                          self(plugin) {
                }


                void initialize(){

                }

                steemit::chain::database &database() {
                    return database_;
                }

                void pre_operation(const operation_notification &op_obj);

                void post_operation(const operation_notification &op_obj);

                void clear_cache();

                void cache_auths(const account_authority_object &a);

                flat_set<public_key_type> cached_keys;
                steemit::chain::database &database_;
                account_by_key_plugin &self;
            };

            struct pre_operation_visitor {
                account_by_key_plugin &_plugin;

                pre_operation_visitor(account_by_key_plugin &plugin)
                        : _plugin(plugin) {
                }

                typedef void result_type;

                template<typename T>
                void operator()(const T &) const {
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const account_create_operation <Major, Hardfork, Release> &op) const {
                    _plugin.my->clear_cache();
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const account_update_operation <Major, Hardfork, Release> &op) const {
                    _plugin.my->clear_cache();
                    auto acct_itr = _plugin.my->database().find<account_authority_object, by_account>(op.account);
                    if (acct_itr) {
                        _plugin.my->cache_auths(*acct_itr);
                    }
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const recover_account_operation <Major, Hardfork, Release> &op) const {
                    _plugin.my->clear_cache();
                    auto acct_itr = _plugin.my->database().find<account_authority_object, by_account>(
                            op.account_to_recover);
                    if (acct_itr) {
                        _plugin.my->cache_auths(*acct_itr);
                    }
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const pow_operation <Major, Hardfork, Release> &op) const {
                    _plugin.my->clear_cache();
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const pow2_operation <Major, Hardfork, Release> &op) const {
                    _plugin.my->clear_cache();
                }
            };

            struct pow2_work_get_account_visitor {
                typedef const account_name_type *result_type;

                template<typename WorkType>
                result_type operator()(const WorkType &work) const {
                    return &work.input.worker_account;
                }
            };

            struct post_operation_visitor {
                account_by_key_plugin &_plugin;

                post_operation_visitor(account_by_key_plugin &plugin)
                        : _plugin(plugin) {
                }

                typedef void result_type;

                template<typename T>
                void operator()(const T &) const {
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const account_create_operation <Major, Hardfork, Release> &op) const {
                    auto acct_itr = _plugin.my->database().find<account_authority_object, by_account>(op.new_account_name);
                    if (acct_itr) {
                        _plugin.update_key_lookup(*acct_itr);
                    }
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const account_update_operation <Major, Hardfork, Release> &op) const {
                    auto acct_itr = _plugin.my->database().find<account_authority_object, by_account>(op.account);
                    if (acct_itr) {
                        _plugin.update_key_lookup(*acct_itr);
                    }
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const recover_account_operation <Major, Hardfork, Release> &op) const {
                    auto acct_itr = _plugin.my->database().find<account_authority_object, by_account>(
                            op.account_to_recover);
                    if (acct_itr) {
                        _plugin.update_key_lookup(*acct_itr);
                    }
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const pow_operation <Major, Hardfork, Release> &op) const {
                    auto acct_itr = _plugin.my->database().find<account_authority_object, by_account>(op.worker_account);
                    if (acct_itr) {
                        _plugin.update_key_lookup(*acct_itr);
                    }
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const pow2_operation <Major, Hardfork, Release> &op) const {
                    const account_name_type *worker_account = op.work.visit(pow2_work_get_account_visitor());
                    if (worker_account == nullptr) {
                        return;
                    }
                    auto acct_itr = _plugin.my->database().find<account_authority_object, by_account>(*worker_account);
                    if (acct_itr) {
                        _plugin.update_key_lookup(*acct_itr);
                    }
                }

                template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
                void operator()(const hardfork_operation <Major, Hardfork, Release> &op) const {
                    if (op.hardfork_id == STEEMIT_HARDFORK_0_16) {
                        auto &db = _plugin.my->database();

                        for (const std::string &acc : hardfork16::get_compromised_accounts()) {
                            const account_object *account = db.find_account(acc);
                            if (account == nullptr) {
                                continue;
                            }

                            db.create<key_lookup_object>([&](key_lookup_object &o) {
                                o.key = public_key_type("GLS8hLtc7rC59Ed7uNVVTXtF578pJKQwMfdTvuzYLwUi8GkNTh5F6");
                                o.account = account->name;
                            });
                        }
                    }
                }
            };

            void account_by_key_plugin::account_by_key_plugin_impl::clear_cache() {
                cached_keys.clear();
            }

            void account_by_key_plugin::account_by_key_plugin_impl::cache_auths(const account_authority_object &a) {
                for (const auto &item : a.owner.key_auths) {
                    cached_keys.insert(item.first);
                }
                for (const auto &item : a.active.key_auths) {
                    cached_keys.insert(item.first);
                }
                for (const auto &item : a.posting.key_auths) {
                    cached_keys.insert(item.first);
                }
            }

            void account_by_key_plugin::account_by_key_plugin_impl::pre_operation(const operation_notification &note) {
                note.op.visit(pre_operation_visitor(self));
            }

            void account_by_key_plugin::account_by_key_plugin_impl::post_operation(const operation_notification &note) {
                note.op.visit(post_operation_visitor(self));
            }


            account_by_key_plugin::account_by_key_plugin() : my(new account_by_key_plugin_impl(*this)) {
            }

            void account_by_key_plugin::set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg
            ) {
            }

            void account_by_key_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                try {
                    ilog("Initializing account_by_key plugin");
                    chain::database &db = my->database();
                    db.pre_apply_operation.connect(
                            [&](const operation_notification &o) {
                                my->pre_operation(o);
                            }
                    );
                    db.post_apply_operation.connect(
                            [&](const operation_notification &o) {
                                my->post_operation(o);
                            }
                    );

                    //db.add_plugin_index<key_lookup_index>();

                } FC_CAPTURE_AND_RETHROW()
            }

            void account_by_key_plugin::plugin_startup() {}

            void account_by_key_plugin::update_key_lookup(const account_authority_object &a) {
                auto &db = my->database();
                flat_set<public_key_type> new_keys;

                // Construct the set of keys in the account's authority
                for (const auto &item : a.owner.key_auths) {
                    new_keys.insert(item.first);
                }
                for (const auto &item : a.active.key_auths) {
                    new_keys.insert(item.first);
                }
                for (const auto &item : a.posting.key_auths) {
                    new_keys.insert(item.first);
                }

                // For each key that needs a lookup
                for (const auto &key : new_keys) {
                    // If the key was not in the authority, add it to the lookup
                    if (my->cached_keys.find(key) == my->cached_keys.end()) {
                        auto lookup_itr = db.find<key_lookup_object, by_key>(std::make_tuple(key, a.account));

                        if (lookup_itr == nullptr) {
                            db.create<key_lookup_object>([&](key_lookup_object &o) {
                                o.key = key;
                                o.account = a.account;
                            });
                        }
                    } else {
                        // If the key was already in the auths, remove it from the set so we don't delete it
                        my->cached_keys.erase(key);
                    }
                }

                // Loop over the keys that were in authority but are no longer and remove them from the lookup
                for (const auto &key : my->cached_keys) {
                    auto lookup_itr = db.find<key_lookup_object, by_key>(std::make_tuple(key, a.account));

                    if (lookup_itr != nullptr) {
                        db.remove(*lookup_itr);
                    }
                }

                my->cached_keys.clear();
            }
        }
    } // steemit::account_by_key
}