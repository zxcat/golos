#ifndef GOLOS_FEED_HISTORY_API_OBJ_HPP
#define GOLOS_FEED_HISTORY_API_OBJ_HPP

#include <chain/include/steemit/chain/steem_objects.hpp>

namespace steemit {
    namespace plugins {
        namespace database_api {

            struct feed_history_api_obj {
                feed_history_api_obj(const chain::feed_history_object &f) :
                        id(f.id),
                        current_median_history(f.current_median_history),
                        price_history(f.price_history.begin(), f.price_history.end()) {
                }

                feed_history_api_obj() {
                }

                feed_history_object::id_type id;
                price current_median_history;
                deque<price> price_history;
            };}}}

FC_REFLECT(steemit::plugins::database_api::feed_history_api_obj,
(id)
        (current_median_history)
        (price_history)
)
#endif //GOLOS_FEED_HISTORY_API_OBJ_HPP
