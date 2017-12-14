#include <golos/plugins/account_statistics/plugin.hpp>
#include <golos/chain/objects/account_object.hpp>
#include <golos/chain/objects/comment_object.hpp>


namespace golos {
    namespace plugins {
        namespace account_statistics {

            using namespace golos::chain;

            struct plugin::plugin_impl {
            public:
                plugin_impl(plugin &plugin) : _self(plugin) {
                }

                virtual ~plugin_impl() {
                }

                void on_operation(const operation_notification &o) {

                }

                plugin &_self;
                flat_set<uint32_t> _tracked_buckets = {60, 3600, 21600, 86400, 604800, 2592000};
                uint32_t _maximum_history_per_bucket_size = 100;
                flat_set<std::string> _tracked_accounts;
            };

            struct operation_process {
                const plugin &_plugin;
                const account_stats_bucket_object &_stats;
                const account_activity_bucket_object &_activity;
                database &_db;

                operation_process(plugin &asp, const account_stats_bucket_object &s,
                                  const account_activity_bucket_object &a) : _plugin(asp), _stats(s), _activity(a),
                        _db(appbase::app().get_plugin<golos::plugins::chain::plugin>().db()) {
                }

                typedef void result_type;

                template<typename T>
                void operator()(const T &) const {
                }
            };

            plugin::plugin() : _my(new plugin_impl(*this)) {

            }

            plugin::~plugin() {
            }

            void plugin::set_program_options(boost::program_options::options_description &cli,
                                             boost::program_options::options_description &cfg) {
                cli.add_options()("account-stats-bucket-size", boost::program_options::value<std::string>()->default_value(
                        "[60,3600,21600,86400,604800,2592000]"),
                                  "Track account statistics by grouping orders into buckets of equal size measured in seconds specified as a JSON array of numbers")(
                        "account-stats-history-per-bucket",
                        boost::program_options::value<uint32_t>()->default_value(100),
                        "How far back in time to track history for each bucker size, measured in the number of buckets (default: 100)")(
                        "account-stats-tracked-accounts", boost::program_options::value<std::string>()->default_value("[]"),
                        "Which accounts to track the statistics of. Empty list tracks all accounts.");
                cfg.add(cli);
            }

            void plugin::plugin_initialize(const boost::program_options::variables_map &options) {
                try {
                    ilog("account_stats plugin: plugin_initialize() begin");

                    database().post_apply_operation.connect([&](const operation_notification &o) {
                        _my->on_operation(o);
                    });

                    ilog("account_stats plugin: plugin_initialize() end");
                } FC_CAPTURE_AND_RETHROW()
            }

            void plugin::plugin_startup() {
                ilog("account_stats plugin: plugin_startup() begin");
                ilog("account_stats plugin: plugin_startup() end");
            }

            const flat_set<uint32_t> &plugin::get_tracked_buckets() const {
                return _my->_tracked_buckets;
            }

            uint32_t plugin::get_max_history_per_bucket() const {
                return _my->_maximum_history_per_bucket_size;
            }

            const flat_set<std::string> &plugin::get_tracked_accounts() const {
                return _my->_tracked_accounts;
            }

        }
    }
} // golos::account_statistics
