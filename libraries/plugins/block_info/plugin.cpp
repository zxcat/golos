#include <golos/chain/database.hpp>

#include <golos/plugins/block_info/block_info.hpp>
#include <golos/plugins/block_info/api.hpp>
#include <golos/plugins/block_info/plugin.hpp>

namespace golos {
    namespace plugins {
        namespace block_info {
            using golos::plugins::block_info_api;
            using namespace golos::chain;

            plugin::plugin() {
            }

            plugin::~plugin() {
            }

            void plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                auto &db = appbase::app().get_plugin<chain::plugin>().db();

                _applied_block_conn = db.applied_block.connect([this](const protocol::signed_block &b) {
                    on_applied_block(b);
                });

                api_ptr = std::make_shared<api>(_block_info);
            }

            void plugin::plugin_startup() {
            }

            void plugin::plugin_shutdown() {
            }

            void plugin::on_applied_block(const protocol::signed_block &b) {
                uint32_t block_num = b.block_num();
                const auto &db = appbase::app().get_plugin<chain::plugin>().db();

                while (block_num >= _block_info.size()) {
                    _block_info.emplace_back();
                }

                block_info &info = _block_info[block_num];
                const dynamic_global_property_object &dgpo = db.get_dynamic_global_properties();

                info.block_id = b.id();
                info.block_size = fc::raw::pack_size(b);
                info.average_block_size = dgpo.average_block_size;
                info.aslot = dgpo.current_aslot;
                info.last_irreversible_block_num = dgpo.last_irreversible_block_num;
                info.num_pow_witnesses = dgpo.num_pow_witnesses;
                return;
            }

        }
    }
} // golos::plugin::block_info

