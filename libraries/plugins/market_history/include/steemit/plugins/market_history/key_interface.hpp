#pragma once

#include <steemit/protocol/asset.hpp>

namespace steemit {
    namespace plugins{
            namespace market_history {
                class key_interface {
                public:
                    key_interface();

                    key_interface(protocol::asset_name_type base, protocol::asset_name_type quote);

                    protocol::asset_name_type base;
                    protocol::asset_name_type quote;
                };
            }
    }
}

FC_REFLECT(steemit::plugins::market_history::key_interface, (base)(quote));