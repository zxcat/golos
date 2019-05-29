#pragma once

#include <golos/plugins/operation_dump/operation_dump_container.hpp>
#include <appbase/plugin.hpp>
#include <golos/chain/database.hpp>
#include <golos/chain/operation_notification.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <queue>

namespace golos { namespace plugins { namespace operation_dump {

namespace bpo = boost::program_options;
using namespace golos::chain;

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

    dump_buffers buffers;
    std::map<uint32_t, std::queue<int64_t>> vote_rshares;
private:
    class operation_dump_plugin_impl;

    std::unique_ptr<operation_dump_plugin_impl> my;
};

} } } //golos::plugins::operation_dump
