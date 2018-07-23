#include <golos/plugins/private_message/private_message_plugin.hpp>
#include <golos/plugins/private_message/private_message_evaluators.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <appbase/application.hpp>

#include <golos/chain/index.hpp>
#include <golos/chain/custom_operation_interpreter.hpp>
#include <golos/chain/generic_custom_operation_interpreter.hpp>

#include <fc/smart_ref_impl.hpp>


//
template<typename T>
T dejsonify(const std::string &s) {
    return fc::json::from_string(s).as<T>();
}

#define LOAD_VALUE_SET(options, name, container, type) \
if( options.count(name) ) { \
    const std::vector<std::string>& ops = options[name].as<std::vector<std::string>>(); \
    std::transform(ops.begin(), ops.end(), std::inserter(container, container.end()), &dejsonify<type>); \
}
//

namespace golos { namespace plugins { namespace private_message {

    class private_message_plugin::private_message_plugin_impl final {
    public:
        private_message_plugin_impl(private_message_plugin& plugin)
            : db_(appbase::app().get_plugin<golos::plugins::chain::plugin>().db()) {

            custom_operation_interpreter_ = std::make_shared
                    <generic_custom_operation_interpreter<private_message::private_message_plugin_operation>>(db_);

            custom_operation_interpreter_->register_evaluator<private_message_evaluator>(&plugin);
            custom_operation_interpreter_->register_evaluator<private_list_evaluator>(&plugin);

            db_.set_custom_operation_interpreter(plugin.name(), custom_operation_interpreter_);
        }

        std::vector<message_api_object> get_inbox(
            const std::string& to, time_point newest, uint16_t limit, std::uint32_t offset) const;

        std::vector<message_api_object> get_outbox(
            const std::string& from, time_point newest, uint16_t limit, std::uint32_t offset) const;

        list_api_object get_list_info(const std::string& owner, const std::string& contact) const;

        list_size_api_object get_list_size(const std::string& owner) const;

        std::vector<list_api_object> get_list(
            const std::string& owner, const private_list_type type, uint16_t limit, uint32_t offset) const;

        ~private_message_plugin_impl() = default;

        bool is_tracked_account(account_name_type) const;

        std::shared_ptr<generic_custom_operation_interpreter<private_message_plugin_operation>> custom_operation_interpreter_;
        flat_map<std::string, std::string> tracked_accounts_;

        golos::chain::database& db_;
    };

    std::vector<message_api_object> private_message_plugin::private_message_plugin_impl::get_inbox(
        const std::string& to, time_point newest, uint16_t limit, std::uint32_t offset
    ) const {
        std::vector<message_api_object> result;
        const auto &idx = db_.get_index<message_index>().indices().get<by_to_date>();

        auto itr = idx.lower_bound(std::make_tuple(to, newest));
        auto etr = idx.upper_bound(std::make_tuple(to, time_point::min()));

        for (; itr != etr && offset; ++itr, --offset);

        result.reserve(limit);
        for (; itr != etr && limit; ++itr, --limit) {
            result.emplace_back(*itr);
        }

        return result;
    }

    std::vector<message_api_object> private_message_plugin::private_message_plugin_impl::get_outbox(
        const std::string& from, time_point newest, uint16_t limit, std::uint32_t offset
    ) const {

        std::vector<message_api_object> result;
        const auto &idx = db_.get_index<message_index>().indices().get<by_from_date>();

        auto itr = idx.lower_bound(std::make_tuple(from, newest));
        auto etr = idx.upper_bound(std::make_tuple(from, time_point::min()));

        for (; itr != etr && offset; ++itr, --offset);

        result.reserve(limit);
        for (; itr != etr && limit; ++itr, --limit) {
            result.emplace_back(*itr);
        }

        return result;
    }

    list_api_object private_message_plugin::private_message_plugin_impl::get_list_info(
        const std::string& owner, const std::string& contact
    ) const {
        const auto& idx = db_.get_index<list_index>().indices().get<by_contact>();
        auto itr = idx.find(std::make_tuple(owner, contact));

        if (itr != idx.end()) {
            list_api_object result(*itr);
            auto ritr = idx.find(std::make_tuple(contact, owner));
            if (idx.end() != ritr) {
                result.contact_type = ritr->type;
            }
            return result;
        }
        return list_api_object();
    }

    list_size_api_object private_message_plugin::private_message_plugin_impl::get_list_size(
        const std::string& owner
    ) const {
        list_size_api_object result;

        const auto& idx = db_.get_index<list_size_index>().indices().get<by_owner>();

        result.owner = owner;
        for (uint8_t i = undefined; i < private_list_type_size; ++i) {
            auto t = static_cast<private_list_type>(i);
            auto itr = idx.find(std::make_tuple(owner, t));
            if (idx.end() != itr) {
                result.info[t] = itr->info;
            } else {
                result.info[t] = list_size_info();
            }
        }

        return result;
    }

    std::vector<list_api_object> private_message_plugin::private_message_plugin_impl::get_list(
        const std::string& owner, const private_list_type type, uint16_t limit, uint32_t offset
    ) const {
        std::vector<list_api_object> result;

        result.reserve(limit);

        const auto& idx = db_.get_index<list_index>().indices().get<by_owner>();
        const auto& ridx = db_.get_index<list_index>().indices().get<by_contact>();
        auto itr = idx.lower_bound(std::make_tuple(owner, type));
        auto etr = idx.upper_bound(std::make_tuple(owner, type));

        for (; itr != etr && offset; ++itr, --offset);

        for (; itr != etr; ++itr) {
            result.emplace_back(*itr);

            auto ritr = ridx.find(std::make_tuple(itr->contact, itr->owner));
            if (ritr != ridx.end()) {
                result.back().contact_type = ritr->type;
            }
        }
        return result;
    }

    void private_message_evaluator::do_apply(const private_message_operation& pm) {
        database& d = db();

        if (!plugin_->is_tracked_account(pm.from) && !plugin_->is_tracked_account(pm.to)) {
            return;
        }

        auto& idx = d.get_index<list_index>().indices().get<by_contact>();
        auto gitr = idx.find(std::make_tuple(pm.to, pm.from));

        GOLOS_CHECK_OP_PARAM(pm, to, {
            d.get_account(pm.to);
            // TODO: fix exception type
            GOLOS_CHECK_VALUE(gitr == idx.end() || gitr->type != ignored, "Sender is in ignored list of receiver");
        });

        d.create<message_object>([&](message_object& pmo) {
            pmo.from = pm.from;
            pmo.to = pm.to;
            pmo.from_memo_key = pm.from_memo_key;
            pmo.to_memo_key = pm.to_memo_key;
            pmo.checksum = pm.checksum;
            pmo.sent_time = pm.sent_time;
            pmo.read_time = time_point_sec::min();
            pmo.receive_time = d.head_block_time();
            pmo.encrypted_message.resize(pm.encrypted_message.size());
            std::copy(
                pm.encrypted_message.begin(), pm.encrypted_message.end(),
                pmo.encrypted_message.begin());
        });

        // Ok, now update contact lists and counters in them

        auto& sidx = d.get_index<list_size_index>().indices().get<by_owner>();

        // Increment counters depends on side of communication
        auto modify_counters = [&](auto& o, const bool is_send) {
            if (is_send) {
                o.total_send_messages++;
                o.unread_send_messages++;
            } else {
                o.total_recv_messages++;
                o.unread_recv_messages++;
            }
        };

        // Update global counters by type of contact

        auto modify_size = [&](auto& owner, auto type, const bool is_new_contact, const bool is_send) {
            auto func = [&](list_size_object& plso) {
                modify_counters(plso.info, is_send);
                if (is_new_contact) {
                    plso.info.total_contacts++;
                }
            };

            auto itr = sidx.find(std::make_tuple(owner, type));
            if (sidx.end() == itr) {
                d.create<list_size_object>([&](list_size_object& plso){
                    plso.owner = owner;
                    plso.type = type;
                    func(plso);
                });
            } else {
                d.modify(*itr, func);
            }
        };

        // Add contact list if it doesn't exist or update it if it exits

        auto modify_contact = [&](auto& owner, auto& contact, auto type, const bool is_send) {
            bool is_new_contact;
            auto itr = idx.find(std::make_tuple(owner, contact));
            if (idx.end() != itr) {
                d.modify(*itr, [&](list_object& plo) {
                    modify_counters(plo, is_send);
                });
                is_new_contact = false;
                type = itr->type;
            } else {
                d.create<list_object>([&](list_object& plo) {
                    plo.owner = owner;
                    plo.contact = contact;
                    plo.type = type;
                    modify_counters(plo, is_send);
                });
                is_new_contact = true;
            }
            modify_size(owner, type, is_new_contact, is_send);
        };

        modify_contact(pm.from, pm.to, pinned, true);
        modify_contact(pm.to, pm.from, undefined, false);
    }

    void private_list_evaluator::do_apply(const private_list_operation& pl) {
        database& d = db();

        if (!plugin_->is_tracked_account(pl.owner) && !plugin_->is_tracked_account(pl.contact)) {
            return;
        }

        GOLOS_CHECK_OP_PARAM(pl, contact, {
            GOLOS_CHECK_VALUE(d.find_account(pl.contact) != nullptr, "Account doesn't exist");
        });

        auto& idx = d.get_index<list_index>().indices().get<by_contact>();
        auto itr = idx.find(std::make_tuple(pl.owner, pl.contact));

        auto& sidx = d.get_index<list_size_index>().indices().get<by_owner>();
        auto ditr = sidx.find(std::make_tuple(pl.owner, pl.type));

        if (idx.end() != itr) {
            GOLOS_CHECK_OP_PARAM(pl, type, {
                GOLOS_CHECK_VALUE(pl.type != itr->type, "Contact already has requested type");
            });

            auto sitr = sidx.find(std::make_tuple(pl.owner, itr->type));

            d.modify(*sitr, [&](list_size_object& src) {
                src.info.total_contacts--;
                src.info.total_send_messages -= itr->total_send_messages;
                src.info.unread_send_messages -= itr->unread_send_messages;
                src.info.total_recv_messages -= itr->total_recv_messages;
                src.info.unread_recv_messages -= itr->unread_recv_messages;
            });

            // if contact is undefined and no messages then remove statistic about it
            if (!sitr->info.total_contacts ||
                (sitr->type == undefined && !sitr->info.total_send_messages && !sitr->info.total_recv_messages)
            ) {
                d.remove(*sitr);
            }

            auto modify_counters = [&](list_size_object& dst) {
                dst.info.total_contacts++;
                dst.info.total_send_messages += itr->total_send_messages;
                dst.info.unread_send_messages += itr->unread_send_messages;
                dst.info.total_recv_messages += itr->total_recv_messages;
                dst.info.unread_recv_messages += itr->unread_recv_messages;
            };

            if (sidx.end() == ditr) {
                d.create<list_size_object>([&](list_size_object& dst) {
                    dst.owner = pl.owner;
                    dst.type = pl.type;
                    modify_counters(dst);
                });
            } else {
                d.modify(*ditr, modify_counters);
            }

            // if contact is undefined and no messages then remove it
            if (pl.type == undefined && !itr->total_send_messages && !itr->total_recv_messages) {
                d.remove(*itr);
            } else {
                d.modify(*itr, [&](list_object& plo) {
                    plo.type = pl.type;
                });
            }
        } else if (pl.type != undefined) {
            d.create<list_object>([&](list_object& plo){
                plo.owner = pl.owner;
                plo.contact = pl.contact;
                plo.type = pl.type;
            });

            if (sidx.end() == ditr) {
                d.create<list_size_object>([&](list_size_object& plso) {
                    plso.owner = pl.owner;
                    plso.type = pl.type;
                    plso.info.total_contacts = 1;
                });
            } else {
                d.modify(*ditr, [&](list_size_object& plso) {
                    plso.info.total_contacts++;
                });
            }
        }
    }

    private_message_plugin::private_message_plugin() = default;

    private_message_plugin::~private_message_plugin() = default;

    const std::string& private_message_plugin::name() {
        static std::string name = "private_message";
        return name;
    }

    void private_message_plugin::set_program_options(
        boost::program_options::options_description &cli,
        boost::program_options::options_description &cfg
    ) {
        cli.add_options()
            ("pm-account-range",
             boost::program_options::value < std::vector < std::string >> ()->composing()->multitoken(),
             "Defines a range of accounts to private messages to/from as a json pair [\"from\",\"to\"] [from,to)");
        cfg.add(cli);
    }

    void private_message_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
        ilog("Intializing private message plugin");
        my = std::make_unique<private_message_plugin::private_message_plugin_impl>(*this);

        add_plugin_index<message_index>(my->db_);
        add_plugin_index<list_index>(my->db_);
        add_plugin_index<list_size_index>(my->db_);

        using pairstring = std::pair<std::string, std::string>;
        LOAD_VALUE_SET(options, "pm-accounts", my->tracked_accounts_, pairstring);
        JSON_RPC_REGISTER_API(name())
    }

    void private_message_plugin::plugin_startup() {
        ilog("Starting up private message plugin");
    }

    void private_message_plugin::plugin_shutdown() {
        ilog("Shuting down private message plugin");
    }

    bool private_message_plugin::private_message_plugin_impl::is_tracked_account(account_name_type name) const {
        if (tracked_accounts_.empty()) {
            return true;
        }

        auto itr = tracked_accounts_.lower_bound(name);
        return tracked_accounts_.end() != itr && name >= itr->first && name <= itr->second;
    }

    bool private_message_plugin::is_tracked_account(account_name_type name) const {
        return my->is_tracked_account(name);
    }

    // Api Defines

    DEFINE_API(private_message_plugin, get_inbox) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 4)

        auto to = args.args->at(0).as<std::string>();
        auto newest = args.args->at(1).as<time_point>();
        auto limit = args.args->at(2).as<uint16_t>();
        auto offset = args.args->at(3).as<std::uint32_t>();
        auto& db = my->db_;

        GOLOS_CHECK_LIMIT(limit, 100);

        return db.with_weak_read_lock([&]() {
            return my->get_inbox(to, newest, limit, offset);
        });
    }

    DEFINE_API(private_message_plugin, get_outbox) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 4)

        auto from = args.args->at(0).as<std::string>();
        auto newest = args.args->at(1).as<time_point>();
        auto limit = args.args->at(2).as<uint16_t>();
        auto offset = args.args->at(3).as<std::uint32_t>();
        auto& db = my->db_;

        GOLOS_CHECK_LIMIT(limit, 100);

        return db.with_weak_read_lock([&]() {
            return my->get_outbox(from, newest, limit, offset);
        });
    }

    DEFINE_API(private_message_plugin, get_list_size) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 1)

        auto owner = args.args->at(0).as<std::string>();
        auto& db = my->db_;

        return db.with_weak_read_lock([&](){
            return my->get_list_size(owner);
        });
    }

    DEFINE_API(private_message_plugin, get_list_info) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 2)

        auto owner = args.args->at(0).as<std::string>();
        auto contact = args.args->at(1).as<std::string>();
        auto& db = my->db_;

        return db.with_weak_read_lock([&](){
            return my->get_list_info(owner, contact);
        });
    }

    DEFINE_API(private_message_plugin, get_list) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 4)

        auto owner = args.args->at(0).as<std::string>();
        auto type = args.args->at(1).as<private_list_type>();
        auto limit = args.args->at(2).as<uint16_t>();
        auto offset = args.args->at(3).as<uint32_t>();
        auto& db = my->db_;

        GOLOS_CHECK_LIMIT(limit, 100);

        return db.with_weak_read_lock([&](){
            return my->get_list(owner, type, limit, offset);
        });
    }

} } } // golos::plugins::private_message
