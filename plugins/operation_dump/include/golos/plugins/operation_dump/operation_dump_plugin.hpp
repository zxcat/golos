#pragma once

#include <appbase/plugin.hpp>
#include <golos/chain/database.hpp>
#include <golos/chain/operation_notification.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <golos/plugins/operation_dump/operation_dump_container.hpp>

namespace golos { namespace plugins { namespace operation_dump {

namespace bpo = boost::program_options;
using namespace golos::chain;

using block_operations = std::map<uint32_t, std::map<uint32_t, operation>>;

class operation_dump_plugin final : public appbase::plugin<operation_dump_plugin> {
public:
    APPBASE_PLUGIN_REQUIRES((chain::plugin))

    operation_dump_plugin();

    ~operation_dump_plugin();

    void set_program_options(bpo::options_description& cli, bpo::options_description& cfg) override;

    void plugin_initialize(const bpo::variables_map& options) override;

    void plugin_startup() override;

    void plugin_shutdown() override;

    static const std::string& name();

    void add_virtual_op(const operation& op, uint32_t block_num);

    block_operations virtual_ops;
    uint32_t applied_op_in_block = 0;
    dump_buffers buffers;
private:
    class operation_dump_plugin_impl;

    std::unique_ptr<operation_dump_plugin_impl> my;
};

} } } //golos::plugins::operation_dump
