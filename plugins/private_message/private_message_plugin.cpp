#include <golos/plugins/private_message/private_message_plugin.hpp>
#include <golos/plugins/private_message/private_message_evaluators.hpp>
#include <golos/plugins/private_message/private_message_objects.hpp>
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
            custom_operation_interpreter_->register_evaluator<private_settings_evaluator>(&plugin);
            custom_operation_interpreter_->register_evaluator<private_contact_evaluator>(&plugin);

            db_.set_custom_operation_interpreter(plugin.name(), custom_operation_interpreter_);
        }

        std::vector<message_api_object> get_inbox(
            const std::string& to, time_point newest, uint16_t limit, std::uint32_t offset) const;

        std::vector<message_api_object> get_outbox(
            const std::string& from, time_point newest, uint16_t limit, std::uint32_t offset) const;

        settings_api_object get_settings(const std::string& owner) const;

        contact_api_object get_contact_item(const contact_object& o) const;

        contact_api_object get_contact_info(const std::string& owner, const std::string& contact) const;

        contacts_size_api_object get_contacts_size(const std::string& owner) const;

        std::vector<contact_api_object> get_contacts(
            const std::string& owner, const private_contact_type, uint16_t limit, uint32_t offset) const;

        ~private_message_plugin_impl() = default;

        bool is_tracked_account(account_name_type) const;

        std::shared_ptr<generic_custom_operation_interpreter<private_message_plugin_operation>> custom_operation_interpreter_;
        flat_map<std::string, std::string> tracked_account_ranges_;
        flat_set<std::string> tracked_account_list_;

        golos::chain::database& db_;
    };

    std::vector<message_api_object> private_message_plugin::private_message_plugin_impl::get_inbox(
        const std::string& to, time_point newest, uint16_t limit, std::uint32_t offset
    ) const {
        std::vector<message_api_object> result;
        const auto& idx = db_.get_index<message_index>().indices().get<by_to_date>();

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
        const auto& idx = db_.get_index<message_index>().indices().get<by_from_date>();

        auto itr = idx.lower_bound(std::make_tuple(from, newest));
        auto etr = idx.upper_bound(std::make_tuple(from, time_point::min()));

        for (; itr != etr && offset; ++itr, --offset);

        result.reserve(limit);
        for (; itr != etr && limit; ++itr, --limit) {
            result.emplace_back(*itr);
        }

        return result;
    }

    settings_api_object private_message_plugin::private_message_plugin_impl::get_settings(
        const std::string& owner
    ) const {
        const auto& idx = db_.get_index<settings_index>().indices().get<by_owner>();
        auto itr = idx.find(owner);
        if (itr != idx.end()) {
            return settings_api_object(*itr);
        }

        return settings_api_object();
    }

    contact_api_object private_message_plugin::private_message_plugin_impl::get_contact_item(
        const contact_object& o
    ) const {
        contact_api_object result(o);

        const auto& idx = db_.get_index<contact_index>().indices().get<by_contact>();
        auto itr = idx.find(std::make_tuple(o.contact, o.owner));

        if (idx.end() != itr) {
            result.remote_type = itr->type;
        }

        return result;
    }

    contact_api_object private_message_plugin::private_message_plugin_impl::get_contact_info(
        const std::string& owner, const std::string& contact
    ) const {
        const auto& idx = db_.get_index<contact_index>().indices().get<by_contact>();
        auto itr = idx.find(std::make_tuple(owner, contact));

        if (itr != idx.end()) {
            return get_contact_item(*itr);
        }
        return contact_api_object();
    }

    contacts_size_api_object private_message_plugin::private_message_plugin_impl::get_contacts_size(
        const std::string& owner
    ) const {
        contacts_size_api_object result;

        const auto& idx = db_.get_index<contact_size_index>().indices().get<by_owner>();

        for (uint8_t i = undefined; i < private_contact_type_size; ++i) {
            auto t = static_cast<private_contact_type>(i);
            auto itr = idx.find(std::make_tuple(owner, t));
            if (idx.end() != itr) {
                result.size[t] = itr->size;
            } else {
                result.size[t] = contacts_size_info();
            }
        }

        return result;
    }

    std::vector<contact_api_object> private_message_plugin::private_message_plugin_impl::get_contacts(
        const std::string& owner, const private_contact_type type, uint16_t limit, uint32_t offset
    ) const {
        std::vector<contact_api_object> result;

        result.reserve(limit);

        const auto& idx = db_.get_index<contact_index>().indices().get<by_owner>();
        auto itr = idx.lower_bound(std::make_tuple(owner, type));
        auto etr = idx.upper_bound(std::make_tuple(owner, type));

        for (; itr != etr && offset; ++itr, --offset);

        for (; itr != etr; ++itr) {
            result.push_back(get_contact_item(*itr));
        }
        return result;
    }

    void private_message_evaluator::do_apply(const private_message_operation& pm) {
        database& d = db();

        if (!plugin_->is_tracked_account(pm.from) && !plugin_->is_tracked_account(pm.to)) {
            return;
        }

        auto& idx = d.get_index<contact_index>().indices().get<by_contact>();
        auto gitr = idx.find(std::make_tuple(pm.to, pm.from));

        auto& tidx = d.get_index<settings_index>().indices().get<by_owner>();
        auto titr = tidx.find(pm.to);

        GOLOS_CHECK_OP_PARAM(pm, to, {
            d.get_account(pm.to);
            // TODO: fix exception type
            GOLOS_CHECK_VALUE(
                gitr == idx.end() || gitr->type != ignored,
                "Sender is in ignore list of receiver");
            // TODO: fix exception type
            GOLOS_CHECK_VALUE(
                titr == tidx.end() || !titr->ignore_messages_from_undefined_contact,
                "Recipient accept messages only from his contact list");
        });

        d.create<message_object>([&](message_object& pmo) {
            pmo.from = pm.from;
            pmo.to = pm.to;
            pmo.nonce = pm.nonce;
            pmo.from_memo_key = pm.from_memo_key;
            pmo.to_memo_key = pm.to_memo_key;
            pmo.checksum = pm.checksum;
            pmo.read_time = time_point_sec::min();
            pmo.receive_time = d.head_block_time();
            pmo.encrypted_message.resize(pm.encrypted_message.size());
            std::copy(
                pm.encrypted_message.begin(), pm.encrypted_message.end(),
                pmo.encrypted_message.begin());
        });

        // Ok, now update contact lists and counters in them

        auto& sidx = d.get_index<contact_size_index>().indices().get<by_owner>();

        // Increment counters depends on side of communication
        auto inc_counters = [&](auto& o, const bool is_send) {
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
            auto modify_counters = [&](auto& plso) {
                inc_counters(plso.size, is_send);
                if (is_new_contact) {
                    plso.size.total_contacts++;
                }
            };

            auto itr = sidx.find(std::make_tuple(owner, type));
            if (sidx.end() == itr) {
                d.create<contact_size_object>([&](auto& plso){
                    plso.owner = owner;
                    plso.type = type;
                    modify_counters(plso);
                });
            } else {
                d.modify(*itr, modify_counters);
            }
        };

        // Add contact list if it doesn't exist or update it if it exits

        auto modify_contact = [&](auto& owner, auto& contact, auto type, const bool is_send) {
            bool is_new_contact;
            auto itr = idx.find(std::make_tuple(owner, contact));
            if (idx.end() != itr) {
                d.modify(*itr, [&](auto& plo) {
                    inc_counters(plo.size, is_send);
                });
                is_new_contact = false;
                type = itr->type;
            } else {
                d.create<contact_object>([&](auto& plo) {
                    plo.owner = owner;
                    plo.contact = contact;
                    plo.type = type;
                    inc_counters(plo.size, is_send);
                });
                is_new_contact = true;
            }
            modify_size(owner, type, is_new_contact, is_send);
        };

        modify_contact(pm.from, pm.to, pinned, true);
        modify_contact(pm.to, pm.from, undefined, false);
    }

    void private_settings_evaluator::do_apply(const private_settings_operation& ps) {
        database& d = db();

        auto& idx = d.get_index<settings_index>().indices().get<by_owner>();
        auto itr = idx.find(ps.owner);

        auto set_settings = [&](settings_object& pso) {
            pso.owner = ps.owner;
            pso.ignore_messages_from_undefined_contact = ps.ignore_messages_from_undefined_contact;
        };

        if (idx.end() != itr) {
            d.modify(*itr, set_settings);
        } else {
            d.create<settings_object>(set_settings);
        }
    }

    void private_contact_evaluator::do_apply(const private_contact_operation& pl) {
        database& d = db();

        if (!plugin_->is_tracked_account(pl.owner) && !plugin_->is_tracked_account(pl.contact)) {
            return;
        }

        auto& contact_idx = d.get_index<contact_index>().indices().get<by_contact>();
        auto contact_itr = contact_idx.find(std::make_tuple(pl.owner, pl.contact));

        GOLOS_CHECK_OP_PARAM(pl, contact, {
            d.get_account(pl.contact);

            if (d.is_producing()) {
                // TODO: fix exception type
                GOLOS_CHECK_VALUE(
                    contact_idx.end() != contact_itr || pl.type != undefined,
                    "Can't add undefined contact");

                // TODO: fix exception type
                std::string json_metadata(contact_itr->json_metadata.begin(), contact_itr->json_metadata.end());
                GOLOS_CHECK_VALUE(
                    contact_itr->type != pl.type || pl.json_metadata != json_metadata,
                    "Contact has the same type");
            }
        });

        auto& owner_idx = d.get_index<contact_size_index>().indices().get<by_owner>();
        auto dst_itr = owner_idx.find(std::make_tuple(pl.owner, pl.type));

        if (contact_idx.end() != contact_itr) {
            auto src_itr = owner_idx.find(std::make_tuple(pl.owner, contact_itr->type));
            if (contact_itr->type != pl.type) {
                // last contact
                if (src_itr->size.total_contacts == 1) {
                    d.remove(*src_itr);
                } else {
                    d.modify(*src_itr, [&](auto& src) {
                        src.size.total_contacts--;
                        src.size -= contact_itr->size;
                    });
                }

                // has messages or type is not undefined
                if (!contact_itr->size.empty() || pl.type != undefined) {
                    auto modify_counters = [&](auto& dst) {
                        dst.size.total_contacts++;
                        dst.size += contact_itr->size;
                    };

                    if (owner_idx.end() == dst_itr) {
                        d.create<contact_size_object>([&](auto& dst) {
                            dst.owner = pl.owner;
                            dst.type = pl.type;
                            modify_counters(dst);
                        });
                    } else {
                        d.modify(*dst_itr, modify_counters);
                    }
                }
            }

            // contact is undefined and no messages
            if (pl.type == undefined && contact_itr->size.empty()) {
                d.remove(*contact_itr);
            } else {
                d.modify(*contact_itr, [&](auto& plo) {
                    plo.type = pl.type;
                    from_string(plo.json_metadata, pl.json_metadata);
                });
            }
        } else if (pl.type != undefined) {
            d.create<contact_object>([&](auto& plo){
                plo.owner = pl.owner;
                plo.contact = pl.contact;
                plo.type = pl.type;
                from_string(plo.json_metadata, pl.json_metadata);
            });

            if (owner_idx.end() == dst_itr) {
                d.create<contact_size_object>([&](auto& plso) {
                    plso.owner = pl.owner;
                    plso.type = pl.type;
                    plso.size.total_contacts = 1;
                });
            } else {
                d.modify(*dst_itr, [&](auto& plso) {
                    plso.size.total_contacts++;
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
        cfg.add_options()
            ("pm-account-range",
             boost::program_options::value<std::vector<std::string>>()->composing()->multitoken(),
             "Defines a range of accounts to private messages to/from as a json pair [\"from\",\"to\"] [from,to]")
            ("pm-account-list",
             boost::program_options::value<std::vector<std::string>>()->composing()->multitoken(),
             "Defines a list of accounts to private messages to/from");
    }

    void private_message_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
        ilog("Intializing private message plugin");
        my = std::make_unique<private_message_plugin::private_message_plugin_impl>(*this);

        add_plugin_index<message_index>(my->db_);
        add_plugin_index<settings_index>(my->db_);
        add_plugin_index<contact_index>(my->db_);
        add_plugin_index<contact_size_index>(my->db_);

        using pairstring = std::pair<std::string, std::string>;
        LOAD_VALUE_SET(options, "pm-account-range", my->tracked_account_ranges_, pairstring);
        auto list = options["pm-account-list"].as<std::vector<std::string>>();
        my->tracked_account_list_.insert(list.begin(), list.end());
        JSON_RPC_REGISTER_API(name())
    }

    void private_message_plugin::plugin_startup() {
        ilog("Starting up private message plugin");
    }

    void private_message_plugin::plugin_shutdown() {
        ilog("Shuting down private message plugin");
    }

    bool private_message_plugin::private_message_plugin_impl::is_tracked_account(account_name_type name) const {
        if (tracked_account_ranges_.empty() && tracked_account_list_.empty()) {
            return true;
        }

        auto list_itr = tracked_account_list_.find(name);
        if (tracked_account_list_.end() != list_itr) {
            return true;
        }

        auto range_itr = tracked_account_ranges_.lower_bound(name);
        return tracked_account_ranges_.end() != range_itr && name >= range_itr->first && name <= range_itr->second;
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

    DEFINE_API(private_message_plugin, get_settings) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 1)

        auto owner = args.args->at(0).as<std::string>();
        auto& db = my->db_;

        return db.with_weak_read_lock([&](){
            return my->get_settings(owner);
        });
    }

    DEFINE_API(private_message_plugin, get_contacts_size) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 1)

        auto owner = args.args->at(0).as<std::string>();
        auto& db = my->db_;

        return db.with_weak_read_lock([&](){
            return my->get_contacts_size(owner);
        });
    }

    DEFINE_API(private_message_plugin, get_contact_info) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 2)

        auto owner = args.args->at(0).as<std::string>();
        auto contact = args.args->at(1).as<std::string>();
        auto& db = my->db_;

        return db.with_weak_read_lock([&](){
            return my->get_contact_info(owner, contact);
        });
    }

    DEFINE_API(private_message_plugin, get_contacts) {
        GOLOS_CHECK_ARGS_COUNT(args.args, 4)

        auto owner = args.args->at(0).as<std::string>();
        auto type = args.args->at(1).as<private_contact_type>();
        auto limit = args.args->at(2).as<uint16_t>();
        auto offset = args.args->at(3).as<uint32_t>();
        auto& db = my->db_;

        GOLOS_CHECK_LIMIT(limit, 100);

        return db.with_weak_read_lock([&](){
            return my->get_contacts(owner, type, limit, offset);
        });
    }

} } } // golos::plugins::private_message
