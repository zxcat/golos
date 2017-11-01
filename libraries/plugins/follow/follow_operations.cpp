#include <steemit/plugins/follow/follow_operations.hpp>

#include <steemit/protocol/operations/operation_utilities_impl.hpp>

namespace steemit {
    namespace plugins {
        namespace follow {

            void follow_operation::validate() const {
                FC_ASSERT(follower != following, "You cannot follow yourself");
            }

            void reblog_operation::validate() const {
                FC_ASSERT(account != author, "You cannot reblog your own content");
            }

        }
    }
} //steemit::follow


namespace fc {

    void to_variant(const steemit::plugins::follow::follow_plugin_operation &var, fc::variant &vo) {
        var.visit(from_operation<from_operation_policy>(vo));
    }

    void from_variant(const fc::variant &var, steemit::plugins::follow::follow_plugin_operation &vo) {
        static std::map<string, uint32_t> to_tag = []() {
            std::map<string, uint32_t> name_map;
            for (int i = 0; i < steemit::plugins::follow::follow_plugin_operation::count(); ++i) {
                steemit::plugins::follow::follow_plugin_operation tmp;
                tmp.set_which(i);
                string n;
                tmp.visit(get_operation_name(n));
                name_map[n] = i;
            }
            return name_map;
        }();

        auto ar = var.get_array();
        if (ar.size() < 2) {
            return;
        }
        if (ar[0].is_uint64()) {
            vo.set_which(ar[0].as_uint64());
        } else {
            std::string operation_name = ar[0].as_string();
            auto itr = to_tag.find(operation_name);
            FC_ASSERT(itr != to_tag.end(), "Invalid operation name: ${n}", ("n", ar[0]));
            vo.set_which(to_tag[operation_name]);
        }
        vo.visit(fc::to_static_variant(ar[1]));
    }
}

namespace steemit {
    namespace protocol {
        void operation_validate(const plugins::follow::follow_plugin_operation &op) {
            op.visit(steemit::protocol::operation_validate_visitor());
        }

        void operation_get_required_authorities(const plugins::follow::follow_plugin_operation &op,
                                                flat_set<protocol::account_name_type> &active,
                                                flat_set<protocol::account_name_type> &owner,
                                                flat_set<protocol::account_name_type> &posting,
                                                std::vector<authority> &other) {
            op.visit(steemit::protocol::operation_get_required_auth_visitor(active, owner, posting, other));
        }
    }
}
