#include <appbase/application.hpp>
#include <golos/chain/database.hpp>

#include <golos/plugins/block_info/api.hpp>
#include <golos/plugins/block_info/block_info.hpp>
#include <golos/plugins/block_info/plugin.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>

#include <golos/plugins/chain/plugin.hpp>


#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

#define GOLOS_BLOCK_INFO_API_PLUGIN_NAME "block_info_api"

namespace golos {
namespace plugins {
namespace block_info_api {
    using boost::container::flat_set;


    class api::api_impl {
    public:
        api_impl(std::vector<block_info> block_info_) : db_(appbase::app().get_plugin<plugins::chain::plugin>().db()), 
            block_info_(block_info_) {
        }

        DECLARE_API(    (get_block_info)        )
        DECLARE_API(    (get_blocks_with_info)  )

        get_block_info_r get_block_info(const get_block_info_a & args);
        get_blocks_with_info_r get_blocks_with_info(const get_blocks_with_info_a & args);

        golos::chain::database &database() {
            return db_;
        }

    private:
        std::vector<block_info> & block_info_;
        golos::chain::database & db_;
    };

    get_block_info_r api::api_impl::get_block_info(const get_block_info_a & args) {
        golos::plugins::block_info_api::get_block_info_r result;
    
		FC_ASSERT(args.start_block_num > 0);
		FC_ASSERT(args.count <= 10000);
		uint32_t n = std::min(uint32_t(block_info_.size()),
		args.start_block_num + args.count);

		for (uint32_t block_num = args.start_block_num;
			block_num < n; block_num++) {
			result.block_info_vec.emplace_back(block_info_[block_num]);
		}

        return result;
    }

    get_blocks_with_info_r api::api_impl::get_blocks_with_info(const get_blocks_with_info_a & args) {    
        get_blocks_with_info_r result;
        const auto & db = database();

        FC_ASSERT(args.start_block_num > 0);
        FC_ASSERT(args.count <= 10000);
        uint32_t n = std::min( uint32_t( block_info_.size() ), args.start_block_num + args.count );

        uint64_t total_size = 0;
        for (uint32_t block_num = args.start_block_num;
             block_num < n; block_num++) {
            uint64_t new_size =
                    total_size + block_info_[block_num].block_size;
            if ((new_size > 8 * 1024 * 1024) &&
                (block_num != args.start_block_num)) {
                    break;
            }
            total_size = new_size;
            result.block_with_info_vec.emplace_back();
            result.block_with_info_vec.back().block = *db.fetch_block_by_number(block_num);
            result.block_with_info_vec.back().info = block_info_[block_num];
        }

        return result;
    }


    api::api(std::vector<block_info> & block_info_) {
        my = std::make_shared<api_impl>(block_info_);
        JSON_RPC_REGISTER_API(GOLOS_BLOCK_INFO_API_PLUGIN_NAME);
        // JSON_RPC_REGISTER_API("block_info_api");
    }

    DEFINE_API ( api, get_block_info ) {
        auto tmp = args.args->at(0).as<get_block_info_a>();
        auto &db = my->database();
        return db.with_read_lock([&]() {
            get_block_info_r result;
            result = my->get_block_info(tmp);
            return result;
        });
    }

    DEFINE_API ( api, get_blocks_with_info ) {
        auto tmp = args.args->at(0).as<get_blocks_with_info_a>();
        auto &db = my->database();
        return db.with_read_lock([&]() {
            get_blocks_with_info_r result;
            result = my->get_blocks_with_info(tmp);
            return result;
        });
    }
}
}
} // golos::plugin::block_info_api
