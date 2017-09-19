#include <steemit/plugins/tolstoy_api/tolstoy_api.hpp>
#include <steemit/plugins/json_rpc/json_rpc_plugin.hpp>

namespace steemit {
    namespace plugins {
        namespace tolstoy_api {

            using fc::variant;

            struct tolstoy_api::tolstoy_api_impl {
            public:
                tolstoy_api_impl() : rpc(appbase::app().get_plugin<json_rpc::json_rpc_plugin>()) {}

                variant send(const std::string &api_namespace,
                             const std::string &name_method,
                             std::vector<variant> body) {
                    return rpc.rpc(api_namespace, name_method, body);
                }

            private:
                json_rpc::json_rpc_plugin &rpc;
            };

            tolstoy_api::tolstoy_api() : pimpl(new tolstoy_api_impl) {
                JSON_RPC_REGISTER_API(__name__)
            }

            tolstoy_api::~tolstoy_api() {

            }

        }
    }
}
