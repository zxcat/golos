#include <golos/plugins/private_message/private_message_evaluators.hpp>
#include <golos/plugins/private_message/private_message_plugin.hpp>
#include <golos/chain/generic_custom_operation_interpreter.hpp>
#include <fc/io/json.hpp>
#include <fc/smart_ref_impl.hpp>

namespace golos {
    namespace plugins {
        namespace private_message {

            template<typename T>
            T dejsonify(const std::string &s) {
                return fc::json::from_string(s).as<T>();
            }

#define DEFAULT_VALUE_VECTOR(value) default_value({fc::json::to_string(value)}, fc::json::to_string(value))
#define LOAD_VALUE_SET(options, name, container, type) \
                if( options.count(name) ) { \
                      const std::vector<std::string>& ops = options[name].as<std::vector<std::string>>(); \
                      std::transform(ops.begin(), ops.end(), std::inserter(container, container.end()), &dejsonify<type>); \
                }


            struct private_message_plugin::private_message_plugin_impl {
            public:
                private_message_plugin_impl() : database_(appbase::app().get_plugin<chain::plugin>().db()) {
                }

                void plugin_initialize(private_message_plugin &self) {
                    _custom_operation_interpreter = std::make_shared<
                            generic_custom_operation_interpreter<private_message_plugin_operation>>(database());

                    _custom_operation_interpreter->register_evaluator<private_message_evaluator>(&self);

                    database().set_custom_operation_interpreter(plugin_name, _custom_operation_interpreter);
                }

                ~private_message_plugin_impl() {
                }

                golos::chain::database &database() {
                    return database_;
                }

                flat_map<std::string, std::string> tracked_accounts() const {
                    return _tracked_accounts;
                }

                golos::chain::database &database_;
                std::shared_ptr<generic_custom_operation_interpreter<
                        private_message_plugin_operation>> _custom_operation_interpreter;
                flat_map<std::string, std::string> _tracked_accounts;
            };


            private_message_plugin::private_message_plugin() {
            }

            private_message_plugin::~private_message_plugin() {
            }


            void private_message_plugin::set_program_options(boost::program_options::options_description &cli,
                                                             boost::program_options::options_description &cfg) {
                cli.add_options()("pm-account-range",
                                  boost::program_options::value<std::vector<std::string>>()->composing()->multitoken(),
                                  "Defines a range of accounts to private messages to/from as a json pair [\"from\",\"to\"] [from,to)");
                cfg.add(cli);
            }

            void private_message_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                ilog("Intializing private message plugin");
                my.reset(new private_message_plugin_impl());
                my->plugin_initialize(*this);
                golos::chain::database &db = my->database();
                db.add_plugin_index<message_index>();

                using pairstring= std::pair<std::string, std::string>;
                LOAD_VALUE_SET(options, "pm-accounts", my->_tracked_accounts, pairstring);
            }

            void private_message_plugin::plugin_startup() {
            }

            flat_map<std::string, std::string> private_message_plugin::tracked_accounts() const {
                return my->tracked_accounts();
            }

        }
    }
}
