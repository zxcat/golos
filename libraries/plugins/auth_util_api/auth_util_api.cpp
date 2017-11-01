#include <appbase/application.hpp>

#include <steemit/protocol/authority.hpp>
#include <steemit/protocol/sign_state.hpp>

#include <steemit/chain/account_object.hpp>
#include <steemit/chain/database.hpp>

#include <steemit/plugins/auth_util_api/auth_util_api.hpp>
#include <steemit/plugins/auth_util_api/auth_util_api_plugin.hpp>
// #include <steemit/plugins/auth_util/auth_util_plugin.hpp> //1
// #include <steemit/plugins/auth_util/auth_util_api.hpp> //2

// #include <steemit/plugins/auth_util_api/auth_util_api_plugin.hpp>
#include <steemit/plugins/chain/chain_plugin.hpp>

#include <fc/container/flat.hpp>

namespace steemit { namespace plugins { namespace auth_util_api {

using boost::container::flat_set;

struct auth_util_api::auth_util_api_impl {
   auth_util_api_impl( ) : db_( appbase::app().get_plugin< steemit::plugins::chain::chain_plugin >().db() ) {         
   }

   DECLARE_API( (check_authority_signature) )

   // void check_authority_signature( const check_authority_signature_params& args, check_authority_signature_result& result )


   // std::shared_ptr< steemit::plugin::auth_util::auth_util_plugin > get_plugin();
   steemit::chain::database &database() {
      return db_;
   }

   steemit::chain::database& db_;
};

DEFINE_API(auth_util_api::auth_util_api_impl, check_authority_signature)  {
   steemit::plugins::auth_util_api::check_authority_signature_return result;

   const chain::account_authority_object& acct = db_.get< chain::account_authority_object, chain::by_account >( args.account_name );
   protocol::authority auth;
   if( (args.level == "posting") || (args.level == "p") )
   {
      auth = protocol::authority( acct.posting );
   }
   else if( (args.level == "active") || (args.level == "a") || (args.level == "") )
   {
      auth = protocol::authority( acct.active );
   }
   else if( (args.level == "owner") || (args.level == "o") )
   {
      auth = protocol::authority( acct.owner );
   }
   else
   {
      FC_ASSERT( false, "invalid level specified" );
   }
   flat_set< protocol::public_key_type > signing_keys;
   for( const protocol::signature_type& sig : args.sigs )
   {
      result.keys.emplace_back( fc::ecc::public_key( sig, args.dig, true ) );
      signing_keys.insert( result.keys.back() );
   }

   flat_set< protocol::public_key_type > avail;
   protocol::sign_state ss( signing_keys, [db_]( const std::string& account_name ) -> const protocol::authority
   {
      return protocol::authority(db_.get< chain::account_authority_object, chain::by_account >( account_name ).active );
   }, avail );

   bool has_authority = ss.check_authority( auth );
   FC_ASSERT( has_authority );

   return result;
}


auth_util_api::auth_util_api() {
   my = std::make_shared < auth_util_api_impl > ();
   JSON_RPC_REGISTER_API( STEEMIT_ACCOUNT_BY_KEY_API_PLUGIN_NAME );
}


// check_authority_signature_result auth_util_api::check_authority_signature( check_authority_signature_params args )
DEFINE_API(auth_util_api, check_authority_signature)
{  
   return my->db_.with_read_lock( [&]() {
      check_authority_signature_return result;
      my->check_authority_signature( args, result );
      return result;
   });
}

} } } // steemit::plugin::auth_util
