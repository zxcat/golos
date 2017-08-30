
#include <steemit/chain/database.hpp>

#include <steemit/plugins/block_info/block_info.hpp>
#include <steemit/plugins/block_info/block_info_plugin.hpp>

namespace steemit {
    namespace plugins {
        namespace block_info {

            block_info_plugin::block_info_plugin(){
                name("block_info");
            }

            block_info_plugin::~block_info_plugin() {
            }

            void block_info_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                auto &db = appbase::app().get_plugin<steemit::plugins::chain::chain_plugin>().db();

                _applied_block_conn = db.applied_block.connect([this](const chain::signed_block &b) { on_applied_block(b); });
            }

            void block_info_plugin::plugin_startup() {
            }

            void block_info_plugin::plugin_shutdown() {
            }

            void block_info_plugin::on_applied_block(const chain::signed_block &b) {
                uint32_t block_num = b.block_num();
                const auto &db = appbase::app().get_plugin<steemit::plugins::chain::chain_plugin>().db();

                while (block_num >= _block_info.size()) {
                    _block_info.emplace_back();
                }

                block_info &info = _block_info[block_num];
                const chain::dynamic_global_property_object &dgpo = db.get_dynamic_global_properties();

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
} // steemit::plugin::block_info

