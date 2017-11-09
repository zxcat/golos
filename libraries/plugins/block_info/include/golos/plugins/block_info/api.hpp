#pragma once

#include <golos/plugins/block_info/block_info.hpp>

#include <golos/protocol/types.hpp>
#include <golos/plugins/json_rpc/utility.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>

namespace golos {
namespace plugins {
namespace block_info_api {
    using golos::plugins::block_info::block_info;
    using golos::plugins::block_info::block_with_info;
    using golos::plugins::json_rpc::msg_pack;

    struct get_block_info_a {
        uint32_t start_block_num = 0;
        uint32_t count = 1000;
    };

    struct get_blocks_with_info_a {
        uint32_t start_block_num = 0;
        uint32_t count = 1000;
    };

    

    struct get_block_info_r {
        std::vector<block_info> block_info_vec;
    };

    struct get_blocks_with_info_r {
        std::vector<block_with_info> block_with_info_vec;
    };

    DEFINE_API_ARGS ( get_block_info,           msg_pack,       get_block_info_r       )
    DEFINE_API_ARGS ( get_blocks_with_info,     msg_pack,       get_blocks_with_info_r )

    class api final {
    public:
        api(std::vector<block_info> & block_info_);

        ~api() = default;

        DECLARE_API ( (get_block_info)        )
        DECLARE_API ( (get_blocks_with_info)  )

    protected:
        class api_impl;
        std::shared_ptr<api_impl> my;
    };

} } } //golos::plugins::block_info_api

FC_REFLECT((golos::plugins::block_info_api::get_block_info_a),
        (start_block_num)(count)
)

FC_REFLECT((golos::plugins::block_info_api::get_blocks_with_info_a),
        (start_block_num)(count)
)

FC_REFLECT((golos::plugins::block_info_api::get_block_info_r),
        (block_info_vec)
)

FC_REFLECT((golos::plugins::block_info_api::get_blocks_with_info_r),
        (block_with_info_vec)
)
