#include <steemit/plugins/private_message/private_message_evaluators.hpp>
#include <steemit/plugins/private_message/private_message_plugin.hpp>
#include <steemit/chain/generic_custom_operation_interpreter.hpp>

#include <fc/smart_ref_impl.hpp>

namespace steemit {
    namespace plugins {
        namespace private_message {

                template<typename T>
                T dejsonify(const string &s) {
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
                    private_message_plugin_impl():database_(appbase::app().get_plugin<steemit::plugins::chain::chain_plugin>().db()){}


                    void plugin_initialize(private_message_plugin& self){
                        _custom_operation_interpreter = std::make_shared<generic_custom_operation_interpreter <private_message_plugin_operation>>(database());

                        _custom_operation_interpreter->register_evaluator<private_message_evaluator>(&self);

                        database().set_custom_operation_interpreter(__name__, _custom_operation_interpreter);
                    }

                    ~private_message_plugin_impl(){}

                    chain::database &database() {
                        return database_;
                    }

                    flat_map<string, string> tracked_accounts() const {
                        return _tracked_accounts;
                    }

                    chain::database &database_;
                    std::shared_ptr<generic_custom_operation_interpreter <private_message_plugin_operation>> _custom_operation_interpreter;
                    flat_map<string, string> _tracked_accounts;
                };


            private_message_plugin::private_message_plugin() {
            }

            private_message_plugin::~private_message_plugin() {
            }


            void private_message_plugin::set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg
            ) {
                cli.add_options()
                        ("pm-account-range",
                         boost::program_options::value<std::vector<std::string>>()->composing()->multitoken(),
                         "Defines a range of accounts to private messages to/from as a json pair [\"from\",\"to\"] [from,to)");
                cfg.add(cli);
            }

            void private_message_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                ilog("Intializing private message plugin");
                my.reset(new private_message_plugin_impl());
                my->plugin_initialize(*this);
                chain::database &db = my->database();
                db.add_plugin_index<message_index>();

                using pairstring= pair<string, string> ;
                LOAD_VALUE_SET(options, "pm-accounts", my->_tracked_accounts, pairstring);
            }

            void private_message_plugin::plugin_startup() {
            }

            flat_map<string, string> private_message_plugin::tracked_accounts() const {
                return my->tracked_accounts();
            }

        }
    }
}



DEFINE_OPERATION_TYPE(steemit::plugins::private_message::private_message_plugin_operation)
