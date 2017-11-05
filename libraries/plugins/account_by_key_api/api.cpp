#include <golos/plugins/account_by_key_api/plugin.hpp>
#include <golos/plugins/account_by_key_api/api.hpp>

#include <golos/plugins/account_by_key/objects.hpp>

namespace golos {
    namespace plugins {
        namespace account_by_key {

            namespace detail {

                class api_impl {
                public:
                    api_impl() : _db(appbase::app().get_plugin<chain::plugin>().db()) {
                    }

                    get_key_references_return get_key_references(const get_key_references_args &args) const;

                    chain::database &_db;
                };

                get_key_references_return api_impl::get_key_references(const get_key_references_args &args) const {
                    get_key_references_return final_result;
                    final_result.accounts.reserve(args.keys.size());

                    const auto &key_idx = _db.get_index<account_by_key::key_lookup_index>().indices().get<
                            account_by_key::by_key>();

                    for (auto &key : args.keys) {
                        std::vector<golos::protocol::account_name_type> result;
                        auto lookup_itr = key_idx.lower_bound(key);

                        while (lookup_itr != key_idx.end() && lookup_itr->key == key) {
                            result.push_back(lookup_itr->account);
                            ++lookup_itr;
                        }

                        final_result.accounts.emplace_back(std::move(result));
                    }

                    return final_result;
                }

            } // detail

            api::api() : my(new detail::api_impl()) {
                JSON_RPC_REGISTER_API(STEEMIT_ACCOUNT_BY_KEY_API_PLUGIN_NAME);
            }

            api::~api() {
            }

            get_key_references_return api::get_key_references(const get_key_references_args &args) {
                return my->_db.with_read_lock([&]() {
                    return my->get_key_references(args);
                });
            }
        }
    }
} // golos::plugins::account_by_key
