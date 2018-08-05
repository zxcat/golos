/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <boost/program_options.hpp>

#include <appbase/application.hpp>

#include <golos/plugins/chain/plugin.hpp>
#include <golos/plugins/json_rpc/utility.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>
#include <golos/plugins/operation_history/plugin.hpp>
#include <golos/plugins/operation_history/applied_operation.hpp>
#include <golos/plugins/account_history/history_object.hpp>


namespace golos { namespace plugins { namespace account_history {
    namespace bpo = boost::program_options;
    using namespace chain;
    using plugins::json_rpc::msg_pack;

    using history_operations = std::map<uint32_t, operation_history::applied_operation>;

    DEFINE_API_ARGS(get_account_history, msg_pack, history_operations)
    DEFINE_API_ARGS(filter_account_history, msg_pack, history_operations)

   /**
    *  This plugin is designed to track a range of operations by account so that one node
    *  doesn't need to hold the full operation history in memory.
    */
    class plugin final: public appbase::plugin<plugin> {
    public:
        APPBASE_PLUGIN_REQUIRES(
            (json_rpc::plugin)
            (chain::plugin)
            (operation_history::plugin)
        )

        plugin();
        ~plugin();

        static const std::string& name();

        void set_program_options(bpo::options_description& cli, bpo::options_description& cfg) override;
        void plugin_initialize(const bpo::variables_map& options) override;
        void plugin_startup() override;
        void plugin_shutdown() override;

        fc::flat_map<std::string, std::string> tracked_accounts() const; /// map start_range to end_range

        DECLARE_API(
            /**
             *  Account operations have sequence numbers from 0 to N where N is the most recent operation.
             *  This method returns operations in the range [from-limit, from]
             *
             *  @param account - name of account, which history requested.
             *  @param from - the absolute sequence number, -1 means most recent, limit is the number of operations before from.
             *  @param limit - the maximum number of items that can be queried (0 to 1000], must be less than from
             */
            (get_account_history)

            /**
             *  Account operations have sequence numbers from 0 to N where N is the most recent operation.
             *  This method returns operations of specified account filtered with query
             *
             *  @param query - filtering query - object with following optional fields:
             *    {
             *      account - name of account, which history requested. mandatory field
             *      limit — limit size of result array (optional, 100 if not set, max 1000)
             *      from - sequence number to begin from (required either `from` or `from_timestamp`)
             *      to - sequence number to end at; it's possible result don't reach it if limited (optional)
             *      from_timestamp - timestamp to begin from (optional, can't be used if `from` set)
             *      to_timestamp - timestamp to end at (optional, can't be used if `to` set)
             *      select_ops - list of operations to include. special values: ALL, REAL, VIRTUAL. if skipped = ALL
             *      filter_ops - blacklist. if skipped = empty list (nothing blacklisted)
             *      dir - direction of operation in relation to account: any, sender, receiver, dual. Experimental
             *    }
             */
            (filter_account_history)
        )

    private:
        struct plugin_impl;
        std::unique_ptr<plugin_impl> pimpl;
    };

} } } // golos::plugins::account_history
