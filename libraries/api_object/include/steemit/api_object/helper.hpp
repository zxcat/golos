#pragma once

#include <steemit/chain/evaluator.hpp>
#include <fc/io/json.hpp>


#include <memory>

namespace steemit {
    namespace application {

/// @group Some useful tools for boost::program_options arguments using vectors of JSON strings
/// @{
        template<typename T>
        T dejsonify(const string &s) {
            return fc::json::from_string(s).as<T>();
        }

#define DEFAULT_VALUE_VECTOR(value) default_value({fc::json::to_string(value)}, fc::json::to_string(value))
#define LOAD_VALUE_SET(options, name, container, type) \
if( options.count(name) ) { \
      const std::vector<std::string>& ops = options[name].as<std::vector<std::string>>(); \
      std::transform(ops.begin(), ops.end(), std::inserter(container, container.end()), &steemit::application::dejsonify<type>); \
}


    }
}

#define STEEMIT_DEFINE_PLUGIN(plugin_name, plugin_class) \
   namespace steemit { namespace plugin { \
   std::shared_ptr< steemit::application::abstract_plugin > create_ ## plugin_name ## _plugin( application::application* application )  \
   { return std::make_shared< plugin_class >( application ); } \
   } }

#define DEFINE_PLUGIN_EVALUATOR(PLUGIN, OPERATION, X)                     \
class X ## _evaluator : public steemit::chain::evaluator< X ## _evaluator, OPERATION > \
{                                                                           \
   public:                                                                  \
      typedef X ## _operation operation_type;                               \
                                                                            \
      X ## _evaluator( chain::database& db, PLUGIN* plugin )                       \
         : steemit::chain::evaluator< X ## _evaluator, OPERATION >( db ), \
           _plugin( plugin )                                                \
      {}                                                                    \
                                                                            \
      void do_apply( const X ## _operation& o );                            \
                                                                            \
      PLUGIN* _plugin;                                                      \
};
