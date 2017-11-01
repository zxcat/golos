#pragma once
#include <steemit/plugins/json_rpc/utility.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

#include <steemit/protocol/types.hpp>

#include <fc/api.hpp>
#include <fc/crypto/sha256.hpp>

#include <string>

namespace steemit { namespace plugins { namespace auth_util_api {

struct check_authority_signature_args
{
   std::string                             account_name;
   std::string                             level;
   fc::sha256                              dig;
   std::vector< protocol::signature_type > sigs;
};

struct check_authority_signature_return
{
   std::vector< protocol::public_key_type > keys;
};

class auth_util_api final {   
public:
   auth_util_api();
   ~auth_util_api() = default;

   DECLARE_API( (check_authority_signature) )

   // check_authority_signature_result check_authority_signature( check_authority_signature_params args );

private:
   struct auth_util_api_impl;
   std::shared_ptr < auth_util_api_impl > my;
};

} } } // steemit::plugins::auth_util_api

FC_REFLECT( (steemit::plugins::auth_util_api::check_authority_signature_args),
   (account_name)
   (level)
   (dig)
   (sigs)
)

FC_REFLECT( (steemit::plugins::auth_util_api::check_authority_signature_return),
   (keys)
)

FC_API( steemit::plugins::auth_util_api::auth_util_api,
   (check_authority_signature)
)