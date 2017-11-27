#include <appbase/application.hpp>
#include <golos/protocol/types.hpp>


#include <fc/log/logger_config.hpp>
#include <golos/utilities/key_conversion.hpp>
#include <fc/git_revision.hpp>

#include <golos/plugins/chain/plugin.hpp>
#include <golos/plugins/p2p/p2p_plugin.hpp>
#include <golos/plugins/webserver/webserver_plugin.hpp>
#include <golos/plugins/network_broadcast_api/network_broadcast_api_plugin.hpp>
#include <golos/plugins/tags/tags_plugin.hpp>
#include <golos/plugins/languages/plugin.hpp>
#include <golos/plugins/account_history/plugin.hpp>
#include <golos/plugins/account_by_key/plugin.hpp>
#include <golos/plugins/witness/witness.hpp>
#include <golos/plugins/follow/plugin.hpp>
#include <golos/plugins/market_history/plugin.hpp>
#include <golos/plugins/database_api/plugin.hpp>
#include <golos/plugins/test_api/test_api_plugin.hpp>
#include <golos/plugins/tolstoy_api/tolstoy_api_plugin.hpp>
#include <golos/plugins/market_history_api/api_plugin.hpp>


#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>
#include <fc/interprocess/signals.hpp>
#include <fc/git_revision.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <csignal>
#include <vector>
#include <fc/log/console_appender.hpp>
#include <fc/log/json_console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>


namespace bpo = boost::program_options;
using golos::protocol::version;


std::string& version_string() {
    static std::string v_str =
            "steem_blockchain_version: " + fc::string( STEEMIT_BLOCKCHAIN_VERSION ) + "\n" +
            "steem_git_revision:       " + fc::string( fc::git_revision_sha )       + "\n" +
            "fc_git_revision:          " + fc::string( fc::git_revision_sha )       + "\n";
    return v_str;
}

namespace golos {

    namespace utilities {
        void set_logging_program_options(boost::program_options::options_description &);
        fc::optional<fc::logging_config> load_logging_config( const boost::program_options::variables_map&, const boost::filesystem::path&);

        struct console_appender_args {
            std::string appender;
            std::string stream;
        };

        struct file_appender_args {
            std::string appender;
            std::string file;
        };

        struct logger_args {
            std::string name;
            std::string level;
            std::string appender;
        };
    }

    namespace plugins {
        void register_plugins();
    }

}

void logo(){

#ifdef BUILD_GOLOS_TESTNET
        std::cerr << "------------------------------------------------------\n\n";
        std::cerr << "            STARTING TEST NETWORK\n\n";
        std::cerr << "------------------------------------------------------\n";
        auto initminer_private_key = golos::utilities::key_to_wif( STEEMIT_INIT_PRIVATE_KEY );
        std::cerr << "initminer public key: " << STEEMIT_INIT_PUBLIC_KEY_STR << "\n";
        std::cerr << "initminer private key: " << initminer_private_key << "\n";
        std::cerr << "chain id: " << std::string( STEEMIT_CHAIN_ID ) << "\n";
        std::cerr << "blockchain version: " << fc::string( STEEMIT_BLOCKCHAIN_VERSION ) << "\n";
        std::cerr << "------------------------------------------------------\n";
#else
    std::cerr << "------------------------------------------------------\n\n";
    std::cerr << "            STARTING GOLOS NETWORK\n\n";
    std::cerr << "------------------------------------------------------\n";
    std::cerr << "initminer public key: " << STEEMIT_INIT_PUBLIC_KEY_STR << "\n";
    std::cerr << "chain id: " << std::string( STEEMIT_CHAIN_ID ) << "\n";
    std::cerr << "blockchain version: " << fc::string( STEEMIT_BLOCKCHAIN_VERSION ) << "\n";
    std::cerr << "------------------------------------------------------\n";
#endif

}

int main( int argc, char** argv ) {
    try {

        // Setup logging config
        boost::program_options::options_description options;

        golos::utilities::set_logging_program_options( options );
        appbase::app().add_program_options( boost::program_options::options_description(), options );

        golos::plugins::register_plugins();
        appbase::app().set_version_string( version_string() );

        bool initialized = appbase::app().initialize<
                golos::plugins::chain::plugin,
                golos::plugins::p2p::p2p_plugin,
                golos::plugins::webserver::webserver_plugin
        >
                ( argc, argv );

        logo();

        if( !initialized )
            return 0;

        auto& args = appbase::app().get_args();

        try {
            fc::optional< fc::logging_config > logging_config = golos::utilities::load_logging_config( args, appbase::app().data_dir() );
            if( logging_config )
                fc::configure_logging( *logging_config );
        } catch( const fc::exception& ) {
            wlog( "Error parsing logging config" );
        }

        appbase::app().startup();
        appbase::app().exec();
        ilog("exited cleanly");
        return 0;
    }
    catch ( const boost::exception& e ) {
        std::cerr << boost::diagnostic_information(e) << "\n";
    }
    catch ( const fc::exception& e ) {
        std::cerr << e.to_detail_string() << "\n";
    }
    catch ( const std::exception& e ) {
        std::cerr << e.what() << "\n";
    }
    catch ( ... ) {
        std::cerr << "unknown exception\n";
    }

    return -1;
}



namespace golos {
    namespace utilities{
        using std::string;
        using std::vector;

        void set_logging_program_options( boost::program_options::options_description& options ) {
            std::vector< std::string > default_console_appender( { "{\"appender\":\"stderr\",\"stream\":\"std_error\"}" } );
            std::string str_default_console_appender = boost::algorithm::join( default_console_appender, " " );

            std::vector< std::string > default_file_appender( { "{\"appender\":\"p2p\",\"file\":\"logs/p2p/p2p.log\"}" } );
            std::string str_default_file_appender = boost::algorithm::join( default_file_appender, " " );

            std::vector< std::string > default_logger(
                    { "{\"name\":\"default\",\"level\":\"warn\",\"appender\":\"stderr\"}\n",
                      "log-logger = {\"name\":\"p2p\",\"level\":\"warn\",\"appender\":\"p2p\"}" } );
            std::string str_default_logger = boost::algorithm::join( default_logger, " " );

            options.add_options()
                    ("log-console-appender", boost::program_options::value< std::vector< std::string > >()->composing()->default_value( default_console_appender, str_default_console_appender ),
                     "Console appender definition json: {\"appender\", \"stream\"}" )
                    ("log-file-appender", boost::program_options::value< std::vector< std::string > >()->composing()->default_value( default_file_appender, str_default_file_appender ),
                     "File appender definition json:  {\"appender\", \"file\"}" )
                    ("log-logger", boost::program_options::value< std::vector< std::string > >()->composing()->default_value( default_logger, str_default_logger ),
                     "Logger definition json: {\"name\", \"level\", \"appender\"}" );
        }

        fc::optional<fc::logging_config> load_logging_config( const boost::program_options::variables_map& args, const boost::filesystem::path& pwd ) {
            try {
                fc::logging_config logging_config;
                bool found_logging_config = false;

                if( args.count( "log-console-appender" ) ) {
                    std::vector< string > console_appenders = args["log-console-appender"].as< vector< string > >();

                    for( string& s : console_appenders ) {
                        try {
                            auto console_appender = fc::json::from_string( s ).as< utilities::console_appender_args >();
                            fc::console_appender::config console_appender_config;
                            console_appender_config.level_colors.emplace_back(fc::console_appender::level_color( fc::log_level::debug, fc::console_appender::color::green));
                            console_appender_config.level_colors.emplace_back(fc::console_appender::level_color( fc::log_level::warn, fc::console_appender::color::brown));
                            console_appender_config.level_colors.emplace_back(fc::console_appender::level_color( fc::log_level::error, fc::console_appender::color::red));
                            console_appender_config.stream = fc::variant( console_appender.stream ).as< fc::console_appender::stream::type >();
                            logging_config.appenders.push_back(fc::appender_config( console_appender.appender, "console", fc::variant( console_appender_config ) ) );
                            found_logging_config = true;
                        }

                        catch( ... ) {

                        }
                    }
                }

                if( args.count( "log-file-appender" ) ) {
                    std::vector< string > file_appenders = args["log-file-appender"].as< std::vector< string > >();

                    for( string& s : file_appenders ) {
                        auto file_appender = fc::json::from_string( s ).as< golos::utilities::file_appender_args>();

                        fc::path file_name = file_appender.file;
                        if( file_name.is_relative() )
                            file_name = fc::absolute( pwd ) / file_name;

                        // construct a default file appender config here
                        // filename will be taken from ini file, everything else hard-coded here
                        fc::file_appender::config file_appender_config;
                        file_appender_config.filename = file_name;
                        file_appender_config.flush = true;
                        file_appender_config.rotate = true;
                        file_appender_config.rotation_interval = fc::hours(1);
                        file_appender_config.rotation_limit = fc::days(1);
                        logging_config.appenders.push_back(
                                fc::appender_config( file_appender.appender, "file", fc::variant( file_appender_config ) ) );
                        found_logging_config = true;
                    }
                }

                if( args.count( "log-logger" ) ) {
                    std::vector< string > loggers = args[ "log-logger" ].as< std::vector< std::string > >();

                    for( string& s : loggers ) {
                        auto logger = fc::json::from_string( s ).as< golos::utilities::logger_args >();

                        fc::logger_config logger_config( logger.name );
                        logger_config.level = fc::variant( logger.level ).as< fc::log_level >();
                        boost::split( logger_config.appenders, logger.appender, boost::is_any_of(" ,"), boost::token_compress_on );
                        logging_config.loggers.push_back( logger_config );
                        found_logging_config = true;
                    }
                }

                if( found_logging_config )
                    return logging_config;
                else
                    return fc::optional< fc::logging_config >();
            }
            FC_RETHROW_EXCEPTIONS(warn, "")
        }
    }

    namespace plugins {
        void register_plugins() {
///PLUGIN
            appbase::app().register_plugin< golos::plugins::chain::plugin                                       >();
            appbase::app().register_plugin< golos::plugins::p2p::p2p_plugin                                     >();
            appbase::app().register_plugin< golos::plugins::webserver::webserver_plugin                         >();
            appbase::app().register_plugin< golos::plugins::follow::plugin                                      >();
            appbase::app().register_plugin< golos::plugins::market_history::plugin                              >();
            appbase::app().register_plugin< golos::plugins::account_by_key::plugin                              >();
            appbase::app().register_plugin< golos::plugins::account_history::plugin                             >();
            appbase::app().register_plugin< golos::plugins::languages::plugin                                   >();
            appbase::app().register_plugin< golos::plugins::tags::tags_plugin                                   >();
            appbase::app().register_plugin< golos::plugins::witness_plugin::witness_plugin                      >();
///API
            appbase::app().register_plugin< golos::plugins::network_broadcast_api::network_broadcast_api_plugin >();
            golos::plugins::database_api::register_database_api();
            appbase::app().register_plugin< golos::plugins::test_api::test_api_plugin                           >();
            appbase::app().register_plugin< golos::plugins::tolstoy_api::tolstoy_api_plugin                     >();
            appbase::app().register_plugin< golos::plugins::market_history::api_plugin                          >();

        }
    }
}

FC_REFLECT( (golos::utilities::console_appender_args), (appender)(stream) )
FC_REFLECT( (golos::utilities::file_appender_args), (appender)(file) )
FC_REFLECT( (golos::utilities::logger_args), (name)(level)(appender) )