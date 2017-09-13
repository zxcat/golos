#pragma once

#include <steemit/protocol/transaction.hpp>
#include <steemit/protocol/block.hpp>
#include <steemit/plugins/chain/chain_plugin.hpp>
#include <appbase/application.hpp>

#define STEEM_P2P_PLUGIN_NAME "p2p"

namespace steemit { namespace plugins { namespace p2p {
namespace bpo = boost::program_options;

class p2p_plugin final : public appbase::plugin<p2p_plugin> {
public:
   APPBASE_PLUGIN_REQUIRES((plugins::chain::chain_plugin))
   static const std::string& name() { static std::string name = STEEM_P2P_PLUGIN_NAME; return name; }
   p2p_plugin();
   ~p2p_plugin();

   void set_program_options(bpo::options_description &, bpo::options_description &config_file_options) override;

   void plugin_initialize(const bpo::variables_map& options) override ;
   void plugin_startup() override ;
   void plugin_shutdown()override ;

   void broadcast_block( const steemit::protocol::signed_block& block );
   void broadcast_transaction( const steemit::protocol::signed_transaction& tx );
   void set_block_production( bool producing_blocks );

private:
   std::unique_ptr<class p2p_plugin_impl> my;
};

} } } // steemit::plugins::p2p
