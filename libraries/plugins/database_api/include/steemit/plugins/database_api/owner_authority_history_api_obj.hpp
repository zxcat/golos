#ifndef GOLOS_OWNER_AUTHORITY_HISTORY_API_OBJ_HPP
#define GOLOS_OWNER_AUTHORITY_HISTORY_API_OBJ_HPP

#include <chain/include/steemit/chain/objects/account_object.hpp>

namespace steemit {
    namespace plugins {
        namespace database_api {
            struct owner_authority_history_api_object {
                owner_authority_history_api_object(const chain::owner_authority_history_object &o)
                        :
                        id(o.id),
                        account(o.account),
                        previous_owner_authority(authority(o.previous_owner_authority)),
                        last_valid_time(o.last_valid_time) {
                }

                owner_authority_history_api_object() {
                }

                owner_authority_history_object::id_type id;

                account_name_type account;
                authority previous_owner_authority;
                time_point_sec last_valid_time;
            };
        }}}

FC_REFLECT((steemit::plugins::database_api::owner_authority_history_api_object),
(id)
        (account)
        (previous_owner_authority)
        (last_valid_time)
)
#endif //GOLOS_OWNER_AUTHORITY_HISTORY_API_OBJ_HPP
