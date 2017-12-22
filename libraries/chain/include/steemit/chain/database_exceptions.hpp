#pragma once

#include <steemit/protocol/exceptions.hpp>

#define STEEMIT_DECLARE_OP_BASE_EXCEPTIONS(op_name)                \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _validate_exception,                                 \
      steemit::chain::operation_validate_exception,                  \
      4040000 + 100 * protocol::operation::tag< protocol::op_name ## _operation >::value, \
      #op_name "_operation validation exception"                      \
      )                                                               \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _evaluate_exception,                                 \
      steemit::chain::operation_evaluate_exception,                  \
      4050000 + 100 * protocol::operation::tag< protocol::op_name ## _operation >::value, \
      #op_name "_operation evaluation exception"                      \
      )

#define STEEMIT_DECLARE_OP_VALIDATE_EXCEPTION(exc_name, op_name, seqnum, msg) \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _ ## exc_name,                                       \
      steemit::chain::op_name ## _validate_exception,                \
      4040000 + 100 * protocol::operation::tag< protocol::op_name ## _operation >::value  \
         + seqnum,                                                    \
      msg                                                             \
      )

#define STEEMIT_DECLARE_OP_EVALUATE_EXCEPTION(exc_name, op_name, seqnum, msg) \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _ ## exc_name,                                       \
      steemit::chain::op_name ## _evaluate_exception,                \
      4050000 + 100 * protocol::operation::tag< protocol::op_name ## _operation >::value  \
         + seqnum,                                                    \
      msg                                                             \
      )

#define STEEMIT_DECLARE_INTERNAL_EXCEPTION(exc_name, seqnum, msg)  \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      internal_ ## exc_name,                                          \
      steemit::chain::internal_exception,                            \
      4990000 + seqnum,                                               \
      msg                                                             \
      )

#define STEEMIT_TRY_NOTIFY(signal, ...)                                     \
   try                                                                        \
   {                                                                          \
      signal( __VA_ARGS__ );                                                  \
   }                                                                          \
   catch( const steemit::chain::plugin_exception& e )                         \
   {                                                                          \
      elog( "Caught plugin exception: ${e}", ("e", e.to_detail_string() ) );  \
      throw;                                                                  \
   }                                                                          \
   catch( const fc::exception& e )                                            \
   {                                                                          \
      elog( "Caught exception in plugin: ${e}", ("e", e.to_detail_string() ) ); \
   }                                                                          \
   catch( ... )                                                               \
   {                                                                          \
      wlog( "Caught unexpected exception in plugin" );                        \
   }

namespace steemit {
    namespace chain {

#define FC_DECLARE_DERIVED_EXCEPTION( TYPE, BASE, CODE, WHAT ) \
   class TYPE : public BASE  \
   { \
      public: \
       enum code_enum { \
          code_value = CODE, \
       }; \
       explicit TYPE( int64_t code, const std::string& name_value, const std::string& what_value ) \
       :BASE( code, name_value, what_value ){} \
       explicit TYPE( fc::log_message&& m, int64_t code, const std::string& name_value, const std::string& what_value ) \
       :BASE( std::move(m), code, name_value, what_value ){} \
       explicit TYPE( fc::log_messages&& m, int64_t code, const std::string& name_value, const std::string& what_value )\
       :BASE( std::move(m), code, name_value, what_value ){}\
       explicit TYPE( const fc::log_messages& m, int64_t code, const std::string& name_value, const std::string& what_value )\
       :BASE( m, code, name_value, what_value ){}\
       TYPE( const std::string& what_value, const fc::log_messages& m ) \
       :BASE( m, CODE, BOOST_PP_STRINGIZE(TYPE), what_value ){} \
       TYPE( fc::log_message&& m ) \
       :BASE( fc::move(m), CODE, BOOST_PP_STRINGIZE(TYPE), WHAT ){}\
       TYPE( fc::log_messages msgs ) \
       :BASE( fc::move( msgs ), CODE, BOOST_PP_STRINGIZE(TYPE), WHAT ) {} \
       TYPE( const TYPE& c ) \
       :BASE(c){} \
       TYPE( const BASE& c ) \
       :BASE(c){} \
       TYPE():BASE(CODE, BOOST_PP_STRINGIZE(TYPE), WHAT){}\
       \
       virtual std::shared_ptr<fc::exception> dynamic_copy_exception()const\
       { return std::make_shared<TYPE>( *this ); } \
       virtual NO_RETURN void     dynamic_rethrow_exception()const \
       { if( code() == CODE ) throw *this;\
         else fc::exception::dynamic_rethrow_exception(); \
       } \
   };

   #define FC_DECLARE_EXCEPTION( TYPE, CODE, WHAT ) \
      FC_DECLARE_DERIVED_EXCEPTION( TYPE, fc::exception, CODE, WHAT )

        FC_DECLARE_EXCEPTION(chain_exception, 4000000, "blockchain exception")

        FC_DECLARE_DERIVED_EXCEPTION(database_query_exception, steemit::chain::chain_exception, 4010000, "database query exception")

        FC_DECLARE_DERIVED_EXCEPTION(block_validate_exception, steemit::chain::chain_exception, 4020000, "block validation exception")

        FC_DECLARE_DERIVED_EXCEPTION(transaction_exception, steemit::chain::chain_exception, 4030000, "transaction validation exception")

        FC_DECLARE_DERIVED_EXCEPTION(operation_validate_exception, steemit::chain::chain_exception, 4040000, "operation validation exception")

        FC_DECLARE_DERIVED_EXCEPTION(operation_evaluate_exception, steemit::chain::chain_exception, 4050000, "operation evaluation exception")

        FC_DECLARE_DERIVED_EXCEPTION(utility_exception, steemit::chain::chain_exception, 4060000, "utility method exception")

        FC_DECLARE_DERIVED_EXCEPTION(undo_database_exception, steemit::chain::chain_exception, 4070000, "undo database exception")

        FC_DECLARE_DERIVED_EXCEPTION(unlinkable_block_exception, steemit::chain::chain_exception, 4080000, "unlinkable block")

        FC_DECLARE_DERIVED_EXCEPTION(unknown_hardfork_exception, steemit::chain::chain_exception, 4090000, "chain attempted to apply unknown hardfork")

        FC_DECLARE_DERIVED_EXCEPTION(plugin_exception, steemit::chain::chain_exception, 4100000, "plugin exception")

        FC_DECLARE_DERIVED_EXCEPTION(block_log_exception, steemit::chain::chain_exception, 4110000, "block log exception")

        FC_DECLARE_DERIVED_EXCEPTION(pop_empty_chain, steemit::chain::undo_database_exception, 4070001, "there are no blocks to pop")

        STEEMIT_DECLARE_OP_BASE_EXCEPTIONS(transfer);
//   STEEMIT_DECLARE_OP_EVALUATE_EXCEPTION( from_account_not_whitelisted, transfer, 1, "owner mismatch" )

        STEEMIT_DECLARE_OP_BASE_EXCEPTIONS(account_create);

        STEEMIT_DECLARE_OP_EVALUATE_EXCEPTION(max_auth_exceeded, account_create, 1, "Exceeds max authority fan-out")

        STEEMIT_DECLARE_OP_EVALUATE_EXCEPTION(auth_account_not_found, account_create, 2, "Auth account not found")

        STEEMIT_DECLARE_OP_BASE_EXCEPTIONS(account_update);

        STEEMIT_DECLARE_OP_EVALUATE_EXCEPTION(max_auth_exceeded, account_update, 1, "Exceeds max authority fan-out")

        STEEMIT_DECLARE_OP_EVALUATE_EXCEPTION(auth_account_not_found, account_update, 2, "Auth account not found")

        FC_DECLARE_DERIVED_EXCEPTION(internal_exception, steemit::chain::chain_exception, 4990000, "internal exception")

        STEEMIT_DECLARE_INTERNAL_EXCEPTION(verify_auth_max_auth_exceeded, 1, "Exceeds max authority fan-out")

        STEEMIT_DECLARE_INTERNAL_EXCEPTION(verify_auth_account_not_found, 2, "Auth account not found")

    }
} // steemit::chain


#pragma once

#include <fc/exception/exception.hpp>
#include <steemit/protocol/exceptions.hpp>

namespace steemit {
    namespace chain {


    }
} // steemit::chain
