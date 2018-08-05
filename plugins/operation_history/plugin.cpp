#include <golos/plugins/operation_history/plugin.hpp>
#include <golos/plugins/operation_history/history_object.hpp>

#include <golos/plugins/json_rpc/api_helper.hpp>
#include <golos/protocol/exceptions.hpp>
#include <golos/chain/operation_notification.hpp>

#include <boost/algorithm/string.hpp>

#define STEEM_NAMESPACE_PREFIX "golos::protocol::"
#define OPERATION_POSTFIX "_operation"


namespace golos { namespace plugins { namespace operation_history {

    struct operation_visitor_filter;

    using namespace golos::protocol;
    using namespace golos::chain;

    struct operation_visitor {
        operation_visitor(
            golos::chain::database& db,
            golos::chain::operation_notification& op_note,
            uint32_t start_block)
            : database(db),
              note(op_note),
              start_block(start_block) {
        }

        using result_type = void;

        golos::chain::database& database;
        golos::chain::operation_notification& note;
        uint32_t start_block;

        template<typename Op>
        void operator()(Op&&) const {
            if (start_block <= database.head_block_num()) {
                note.stored_in_db = true;

                database.create<operation_object>([&](operation_object& obj) {
                    note.db_id = obj.id._id;

                    obj.trx_id = note.trx_id;
                    obj.block = note.block;
                    obj.trx_in_block = note.trx_in_block;
                    obj.op_in_trx = note.op_in_trx;
                    obj.virtual_op = note.virtual_op;
                    obj.timestamp = note.timestamp;

                    const auto size = fc::raw::pack_size(note.op);
                    obj.serialized_op.resize(size);
                    fc::datastream<char*> ds(obj.serialized_op.data(), size);
                    fc::raw::pack(ds, note.op);
                });
            }
        }
    };

    struct operation_visitor_filter final : operation_visitor {

        operation_visitor_filter(
            golos::chain::database& db,
            golos::chain::operation_notification& note,
            const fc::flat_set<std::string>& ops_list,
            bool is_blacklist,
            uint32_t block)
            : operation_visitor(db, note, block),
              filter(ops_list),
              blacklist(is_blacklist),
              start_block(block) {
        }

        const fc::flat_set<std::string>& filter;
        bool blacklist;
        uint32_t start_block;

        template <typename T>
        void operator()(const T& op) const {
            if (filter.find(fc::get_typename<T>::name()) != filter.end()) {
                if (!blacklist) {
                    operation_visitor::operator()(op);
                }
            } else {
                if (blacklist) {
                    operation_visitor::operator()(op);
                }
            }
        }
    };

    struct plugin::plugin_impl final {
    public:
        plugin_impl(): database(appbase::app().get_plugin<chain::plugin>().db()) {
        }

        ~plugin_impl() = default;

        void erase_old_blocks() {
            uint32_t head_block = database.head_block_num();
            if (history_blocks <= head_block) {
                uint32_t need_block = head_block - history_blocks;
                const auto& idx = database.get_index<operation_index>().indices().get<by_location>();
                auto it = idx.begin();
                while (it != idx.end() && it->block <= need_block) {
                    auto next_it = it;
                    ++next_it;
                    database.remove(*it);
                    it = next_it;
                }
            }
        }

        void on_operation(golos::chain::operation_notification& note) {
            if (filter_content) {
                note.op.visit(operation_visitor_filter(database, note, ops_list, blacklist, start_block));
            } else {
                note.op.visit(operation_visitor(database, note, start_block));
            }
        }

        std::vector<applied_operation> get_ops_in_block(
            uint32_t block_num,
            bool only_virtual
        ) {
            const auto& idx = database.get_index<operation_index>().indices().get<by_location>();
            auto itr = idx.lower_bound(block_num);
            std::vector<applied_operation> result;
            for (; itr != idx.end() && itr->block == block_num; ++itr) {
                applied_operation operation(*itr);
                if (!only_virtual || operation.virtual_op != 0) {
                    result.push_back(std::move(operation));
                }
            }
            return result;
        }

        annotated_signed_transaction get_transaction(transaction_id_type id) {
            const auto &idx = database.get_index<operation_index>().indices().get<by_transaction_id>();
            auto itr = idx.lower_bound(id);
            if (itr != idx.end() && itr->trx_id == id) {
                auto blk = database.fetch_block_by_number(itr->block);
                FC_ASSERT(blk.valid());
                FC_ASSERT(blk->transactions.size() > itr->trx_in_block);
                annotated_signed_transaction result = blk->transactions[itr->trx_in_block];
                result.block_num = itr->block;
                result.transaction_num = itr->trx_in_block;
                return result;
            }
            GOLOS_THROW_MISSING_OBJECT("transaction", id);
        }

        bool filter_content = false;
        uint32_t start_block = 0;
        uint32_t history_blocks = UINT32_MAX;
        bool blacklist = true;
        fc::flat_set<std::string> ops_list;
        golos::chain::database& database;
    };

    DEFINE_API(plugin, get_ops_in_block) {
        PLUGIN_API_VALIDATE_ARGS(
            (uint32_t, block_num)
            (bool,     only_virtual)
        );
        return pimpl->database.with_weak_read_lock([&](){
            return pimpl->get_ops_in_block(block_num, only_virtual);
        });
    }

    DEFINE_API(plugin, get_transaction) {
        PLUGIN_API_VALIDATE_ARGS(
            (transaction_id_type, id)
        );
        return pimpl->database.with_weak_read_lock([&](){
            return pimpl->get_transaction(id);
        });
    }

    void plugin::set_program_options(
        boost::program_options::options_description& cli,
        boost::program_options::options_description& cfg
    ) {
        cli.add_options() (
            "history-whitelist-ops",
            boost::program_options::value<std::vector<std::string>>()->composing(),
            "Defines a list of operations which will be explicitly logged."
        ) (
            "history-blacklist-ops",
            boost::program_options::value<std::vector<std::string>>()->composing(),
            "Defines a list of operations which will be explicitly ignored."
        ) (
            "history-start-block",
            boost::program_options::value<uint32_t>(),
            "Defines starting block from which recording stats."
        ) (
            "history-blocks",
            boost::program_options::value<uint32_t>(),
            "Defines depth of history for recording stats."
        );
    }

    void plugin::plugin_initialize(const boost::program_options::variables_map& options) {
        ilog("operation_history plugin: plugin_initialize() begin");

        pimpl = std::make_unique<plugin_impl>();

        pimpl->database.pre_apply_operation.connect([&](golos::chain::operation_notification& note){
            pimpl->on_operation(note);
        });

        golos::chain::add_plugin_index<operation_index>(pimpl->database);

        auto split_list = [&](const std::vector<std::string>& ops_list) {
            for (const auto& raw: ops_list) {
                std::vector<std::string> ops;
                boost::split(ops, raw, boost::is_any_of(" \t,"));

                for (const auto& op : ops) {
                    if (op.size()) {
                        std::size_t pos = op.find(OPERATION_POSTFIX);
                        if (pos not_eq std::string::npos and (pos + strlen(OPERATION_POSTFIX)) == op.size()) {
                            pimpl->ops_list.insert(STEEM_NAMESPACE_PREFIX + op);
                        } else {
                            pimpl->ops_list.insert(STEEM_NAMESPACE_PREFIX + op + OPERATION_POSTFIX);
                        }
                    }
                }
            }
        };

        if (options.count("history-whitelist-ops")) {
            GOLOS_CHECK_OPTION(!options.count("history-blacklist-ops"),
                "history-blacklist-ops and history-whitelist-ops can't be specified together");

            pimpl->filter_content = true;
            pimpl->blacklist = false;
            split_list(options.at("history-whitelist-ops").as<std::vector<std::string>>());
            ilog("operation_history: whitelisting ops ${o}", ("o", pimpl->ops_list));
        } else if (options.count("history-blacklist-ops")) {
            pimpl->filter_content = true;
            pimpl->blacklist = true;
            split_list(options.at("history-blacklist-ops").as<std::vector<std::string>>());
            ilog("operation_history: blacklisting ops ${o}", ("o", pimpl->ops_list));
        }

        if (options.count("history-start-block")) {
            pimpl->filter_content = true;
            pimpl->start_block = options.at("history-start-block").as<uint32_t>();
        } else {
            pimpl->start_block = 0;
        }
        ilog("operation_history: start_block ${s}", ("s", pimpl->start_block));

        if (options.count("history-blocks")) {
            uint32_t history_blocks = options.at("history-blocks").as<uint32_t>();
            pimpl->history_blocks = history_blocks;
            pimpl->database.applied_block.connect([&](const signed_block& block){
                pimpl->erase_old_blocks();
            });
        } else {
            pimpl->history_blocks = UINT32_MAX;
        }
        ilog("operation_history: history-blocks ${s}", ("s", pimpl->history_blocks));

        JSON_RPC_REGISTER_API(name());
        ilog("operation_history plugin: plugin_initialize() end");
    }

    plugin::plugin() = default;

    plugin::~plugin() = default;

    const std::string& plugin::name() {
        static std::string name = "operation_history";
        return name;
    }

    void plugin::plugin_startup() {
        ilog("operation_history plugin: plugin_startup() begin");
        ilog("operation_history plugin: plugin_startup() end");
    }

    void plugin::plugin_shutdown() {
    }

} } } // golos::plugins::operation_history
