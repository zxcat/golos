#include <golos/plugins/chain_api/api_plugin.hpp>
#include <golos/plugins/chain_api/api.hpp>

namespace golos {
    namespace plugins {
        namespace chain {
            class api::api_impl {
            public:
                api_impl() : _chain(appbase::app().get_plugin<plugin>()) {
                }

                DECLARE_API((push_block)(push_transaction))

            private:
                plugin &_chain;
            };

            DEFINE_API(chain_api_impl, push_block) {
                push_block_return result;

                result.success = false;

                try {
                    _chain.accept_block(args.block, args.currently_syncing, chain::database::skip_nothing);
                    result.success = true;
                } catch (const fc::exception &e) {
                    result.error = e.to_detail_string();
                } catch (const std::exception &e) {
                    result.error = e.what();
                } catch (...) {
                    result.error = "uknown error";
                }

                return result;
            }

            DEFINE_API(chain_api_impl, push_transaction) {
                push_transaction_return result;

                result.success = false;

                try {
                    _chain.accept_transaction(args);
                    result.success = true;
                } catch (const fc::exception &e) {
                    result.error = e.to_detail_string();
                } catch (const std::exception &e) {
                    result.error = e.what();
                } catch (...) {
                    result.error = "uknown error";
                }

                return result;
            }

            api::api() : my(new api_impl()) {
                JSON_RPC_REGISTER_API(STEEMIT_CHAIN_API_PLUGIN_NAME);
            }

            api::~api() {
            }

            DEFINE_API(chain_api, push_block) {
                return my->push_block(args);
            }

            DEFINE_API(chain_api, push_transaction) {
                return my->push_transaction(args);
            }

        }
    }
} //golos::plugins::chain
