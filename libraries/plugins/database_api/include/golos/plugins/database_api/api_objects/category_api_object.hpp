#ifndef GOLOS_CATEGORY_API_OBJ_HPP
#define GOLOS_CATEGORY_API_OBJ_HPP

#include <golos/chain/objects/comment_object.hpp>

namespace golos {
    namespace plugins {
        namespace database_api {

            struct category_api_object {
                category_api_object(const chain::category_object &c) : id(c.id), name(to_string(c.name)),
                        abs_rshares(c.abs_rshares), total_payouts(c.total_payouts), discussions(c.discussions),
                        last_update(c.last_update) {
                }

                category_api_object() {
                }

                category_object::id_type id;
                string name;
                share_type abs_rshares;
                asset<0, 17, 0> total_payouts;
                uint32_t discussions;
                time_point_sec last_update;
            };
        }
    }
}

FC_REFLECT((golos::plugins::database_api::category_api_obj),
           (id)(name)(abs_rshares)(total_payouts)(discussions)(last_update))

#endif //GOLOS_CATEGORY_API_OBJ_HPP
