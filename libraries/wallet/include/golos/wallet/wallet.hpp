#pragma once

<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
#include <golos/wallet/remote_node_api.hpp>

=======
#include <golos/application/api.hpp>
#include <golos/private_message/private_message_plugin.hpp>
#include <golos/follow/follow_api.hpp>
#include <golos/market_history/market_history_api.hpp>

#include <golos/application/api_objects/steem_api_objects.hpp>

>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp
#include <golos/utilities/key_conversion.hpp>

#include <fc/real128.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/api.hpp>

<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
namespace golos {
    namespace wallet {

        using namespace std;

        using namespace golos::plugins::database_api;
        using namespace golos::plugins;

        using namespace golos::chain::utilities;
        using namespace golos::protocol;
=======
using namespace golos::application;
using namespace golos::chain;
using namespace graphene::utilities;
using namespace std;

namespace golos {
    namespace wallet {

        using golos::application::discussion;
        using namespace golos::private_message;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

        typedef uint16_t transaction_handle_type;

        struct memo_data {

            static optional<memo_data> from_string(std::string str) {
                try {
                    if (str.size() > sizeof(memo_data) && str[0] == '#') {
                        auto data = fc::from_base58(str.substr(1));
                        auto m = fc::raw::unpack<memo_data>(data);
                        FC_ASSERT(std::string(m) == str);
                        return m;
                    }
                } catch (...) {
                }
                return optional<memo_data>();
            }

            public_key_type from;
            public_key_type to;
            uint64_t nonce = 0;
            uint32_t check = 0;
            std::vector<char> encrypted;

            operator std::string() const {
                auto data = fc::raw::pack(*this);
                auto base58 = fc::to_base58(data);
                return '#' + base58;
            }
        };


        struct brain_key_info {
            std::string brain_priv_key;
            public_key_type pub_key;
            std::string wif_priv_key;
        };

        struct wallet_data {
            std::vector<char> cipher_keys; /** encrypted keys */

            std::string ws_server = "ws://localhost:8090";
            std::string ws_user;
            std::string ws_password;
        };

<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
=======
        struct approval_delta {
            std::vector<std::string> active_approvals_to_add;
            std::vector<std::string> active_approvals_to_remove;
            std::vector<std::string> owner_approvals_to_add;
            std::vector<std::string> owner_approvals_to_remove;
            std::vector<std::string> posting_approvals_to_add;
            std::vector<std::string> posting_approvals_to_remove;
            std::vector<std::string> key_approvals_to_add;
            std::vector<std::string> key_approvals_to_remove;
        };

        struct signed_block_with_info : public signed_block {
            signed_block_with_info(const signed_block &block);

            signed_block_with_info(const signed_block_with_info &block) = default;

            block_id_type block_id;
            public_key_type signing_key;
            std::vector<transaction_id_type> transaction_ids;
        };

>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp
        enum authority_type {
            owner, active, posting
        };

        namespace detail {
            class wallet_api_impl;
        }

        /**
         * This wallet assumes it is connected to the database server with a high-bandwidth, low-latency connection and
         * performs minimal caching. This API could be provided locally to be used by a web interface.
         */
        class wallet_api {
        public:
            wallet_api(const wallet_data &initial_data, const golos::protocol::chain_id_type &_steem_chain_id,
                       fc::api<remote_node_api> rapi);

            virtual ~wallet_api();

            bool copy_wallet_file(std::string destination_filename);


            /** Returns a list of all commands supported by the wallet API.
             *
             * This lists each command, along with its arguments and return types.
             * For more detailed help on a single command, use \c get_help()
             *
             * @returns a multi-line std::string suitable for displaying on a terminal
             */
            std::string help() const;

            /**
             * Returns info about the current state of the blockchain
             */
            variant info();

            /** Returns info such as client version, git version of graphene/fc, version of boost, openssl.
             * @returns compile time info and client and dependencies versions
             */
            variant_object about() const;

            /** Returns the information about a block
             *
             * @param num Block num
             *
             * @returns Public block data on the blockchain
             */
            // optional<signed_block> get_block( uint32_t num );

            /** Returns sequence of operations included/generated in a specified block
             *
             * @param block_num Block height of specified block
             * @param only_virtual Whether to only return virtual operations
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp

            // std::vector<golos::plugins::api::applied_operation> get_ops_in_block( uint32_t block_num, bool only_virtual = true );
=======
            std::vector<applied_operation> get_ops_in_block(uint32_t block_num, bool only_virtual = true);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Return the current price feed history
             *
             * @returns Price feed history data on the blockchain
             */
            feed_history_api_object get_feed_history() const;

            /**
             * Returns the list of witnesses producing blocks in the current round (21 Blocks)
             */
            std::vector<account_name_type> get_active_witnesses() const;

            /**
<<<<<<< HEAD
             * Returns the queue of pow miners waiting to produce blocks.
             */
            std::vector<account_name_type> get_miner_queue() const;

            =======
            *
            Returns the
            state info
            associated with
            the URL
            */
            // TODO
            // condenser_api::state get_state( string url );
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

            /**
             * Returns vesting withdraw routes for an account.
             *
             * @param account Account to query routes
             * @param type Withdraw type type [incoming, outgoing, all]
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            // TODO
            // vector< api::withdraw_vesting_route_api_object > get_withdraw_routes( string account, condenser_api::withdraw_route_type type = condenser_api::all )const;
=======
            std::vector<withdraw_route> get_withdraw_routes(std::string account, withdraw_route_type type = all) const;

            /**
            *  Returns the amount of accounts registered in blockchain
            */
            std::string get_account_count() const;

            /**
            *  Gets the steem price per mvests
            */
            std::string get_steem_per_mvests() const;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             *  Gets the account information for all accounts for which this wallet has a private key
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            // vector<golos::plugins::api::account_api_object > list_my_accounts();
=======
            std::vector<account_api_obj> list_my_accounts() const;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Lists all accounts registered in the blockchain.
             * This returns a list of all account names and their account ids, sorted by account name.
             *
             * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all accounts,
             * start by setting \c lowerbound to the empty std::string \c "", and then each iteration, pass
             * the last account name returned as the \c lowerbound for the next \c list_accounts() call.
             *
             * @param lowerbound the name of the first account to return.  If the named account does not exist,
             *                   the list will start at the account that comes after \c lowerbound
             * @param limit the maximum number of accounts to return (max: 1000)
             * @returns a list of accounts mapping account names to account ids
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD set<string>

            list_accounts(const string &lowerbound, uint32_t limit);
=======
            std::set<std::string> list_accounts(const std::string &lowerbound, uint32_t limit);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** List the balances of an account.
             * Each account can have multiple balances, one for each type of asset owned by that
             * account.  The returned list will only contain assets for which the account has a
             * nonzero balance
             * @param account_name the name or id of the account whose balances you want
             * @returns a list of the given account's balances
             */
            std::vector<asset<0, 17, 0>> list_account_balances(const account_name_type &account_name);

            /** Lists all assets registered on the blockchain.
             *
             * To list all assets, pass the empty std::string \c "" for the lowerbound to start
             * at the beginning of the list, and iterate as necessary.
             *
             * @param lowerbound  the symbol of the first asset to include in the list.
             * @param limit the maximum number of assets to return (max: 100)
             * @returns the list of asset objects, ordered by symbol
             */
            std::vector<asset_object> list_assets(const std::string &lowerbound, uint32_t limit) const;

            =======

            vector<account_name_type> list_accounts(const string &lowerbound, uint32_t limit);

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

            /** Returns the block chain's rapidly-changing properties.
             * The returned object contains information that changes every block interval
             * such as the head block number, the next witness, etc.
             * @see \c get_global_properties() for less-frequently changing properties
             * @returns the dynamic global properties
             */
            <<<<<<<
            HEAD dynamic_global_property_object

            get_dynamic_global_properties() const;

            =======

            api::dynamic_global_property_object get_dynamic_global_properties() const;

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

            /** Returns information about the given account.
             *
             * @param account_name the name of the account to provide information about
             * @returns the public account data stored in the blockchain
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            api::account_api_object get_account(string account_name) const;
=======
            account_api_obj get_account(std::string account_name) const;

            /** Returns information about the given asset.
             * @param asset_symbol the symbol of the asset in the request
             * @returns the information about the asset stored in the block chain
             */
            asset_object get_asset(std::string asset_symbol) const;

            /** Returns information about the given proposed_transaction.
             * @param account_name the proposal author name
             * @param id the proposal identification number unique for the account given
             * @returns the information about the asset stored in the block chain
             */
            proposal_object get_proposal(std::string account_name, integral_id_type id) const;

            /** Returns the BitAsset-specific data for a given asset.
             * Market-issued assets's behavior are determined both by their "BitAsset Data" and
             * their basic asset data, as returned by \c get_asset().
             * @param asset_symbol the symbol of the BitAsset in the request
             * @returns the BitAsset-specific data for this asset
             */
            asset_bitasset_data_object get_bitasset_data(std::string asset_symbol) const;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Returns the current wallet filename.
             *
             * This is the filename that will be used when automatically saving the wallet.
             *
             * @see set_wallet_filename()
             * @return the wallet filename
             */
            std::string get_wallet_filename() const;

            /**
             * Get the WIF private key corresponding to a public key.  The
             * private key must already be in the wallet.
             */
            std::string get_private_key(public_key_type pubkey) const;

            /**
             *  @param role - active | owner | posting | memo
             */
            std::pair<public_key_type, std::string> get_private_key_from_password(std::string account, std::string role,
                                                                        std::string password) const;


            /**
             * Returns transaction by ID.
             */
            annotated_signed_transaction get_transaction(transaction_id_type trx_id) const;

            /** Checks whether the wallet has just been created and has not yet had a password set.
             *
             * Calling \c set_password will transition the wallet to the locked state.
             * @return true if the wallet is new
             * @ingroup Wallet Management
             */
            bool is_new() const;

            /** Checks whether the wallet is locked (is unable to use its private keys).
             *
             * This state can be changed by calling \c lock() or \c unlock().
             * @return true if the wallet is locked
             * @ingroup Wallet Management
             */
            bool is_locked() const;

            /** Locks the wallet immediately.
             * @ingroup Wallet Management
             */
            void lock();

            /** Unlocks the wallet.
             *
             * The wallet remain unlocked until the \c lock is called
             * or the program exits.
             * @param password the password previously set with \c set_password()
             * @ingroup Wallet Management
             */
            void unlock(std::string password);

            /** Sets a new password on the wallet.
             *
             * The wallet must be either 'new' or 'unlocked' to
             * execute this command.
             * @ingroup Wallet Management
             */
            void set_password(std::string password);

            /** Dumps all private keys owned by the wallet.
             *
             * The keys are printed in WIF format.  You can import these keys into another wallet
             * using \c import_key()
             * @returns a map containing the private keys, indexed by their public key
             */
            std::map<public_key_type, std::string> list_keys();

            /** Returns detailed help on a single API command.
             * @param method the name of the API command you want help with
             * @returns a multi-line std::string suitable for displaying on a terminal
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            string gethelp(const string &method) const;
=======
            std::string get_help(const std::string &method) const;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Loads a specified Graphene wallet.
             *
             * The current wallet is closed before the new wallet is loaded.
             *
             * @warning This does not change the filename that will be used for future
             * wallet writes, so this may cause you to overwrite your original
             * wallet unless you also call \c set_wallet_filename()
             *
             * @param wallet_filename the filename of the wallet JSON file to load.
             *                        If \c wallet_filename is empty, it reloads the
             *                        existing wallet file
             * @returns true if the specified wallet is loaded
             */
            bool load_wallet_file(std::string wallet_filename = "");

            /** Saves the current wallet to the given filename.
             *
             * @warning This does not change the wallet filename that will be used for future
             * writes, so think of this function as 'Save a Copy As...' instead of
             * 'Save As...'.  Use \c set_wallet_filename() to make the filename
             * persist.
             * @param wallet_filename the filename of the new wallet JSON file to create
             *                        or overwrite.  If \c wallet_filename is empty,
             *                        save to the current filename.
             */
            void save_wallet_file(std::string wallet_filename = "");

            /** Sets the wallet filename used for future writes.
             *
             * This does not trigger a save, it only changes the default filename
             * that will be used the next time a save is triggered.
             *
             * @param wallet_filename the new filename to use for future saves
             */
            void set_wallet_filename(std::string wallet_filename);

            /** Suggests a safe brain key to use for creating your account.
             * \c create_account_with_brain_key() requires you to specify a 'brain key',
             * a long passphrase that provides enough entropy to generate cyrptographic
             * keys.  This function will suggest a suitably random std::string that should
             * be easy to write down (and, with effort, memorize).
             * @returns a suggested brain_key
             */
            brain_key_info suggest_brain_key() const;

            /** Converts a signed_transaction in JSON form to its binary representation.
             *
             * TODO: I don't see a broadcast_transaction() function, do we need one?
             *
             * @param tx the transaction to serialize
             * @returns the binary form of the transaction.  It will not be hex encoded,
             *          this returns a raw std::string that may have null characters embedded
             *          in it
             */
            std::string serialize_transaction(signed_transaction tx) const;

            /** Imports a WIF Private Key into the wallet to be used to sign transactions by an account.
             *
             * example: import_key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
             *
             * @param wif_key the WIF Private Key to import
             */
            bool import_key(std::string wif_key);

            /** Transforms a brain key to reduce the chance of errors when re-entering the key from memory.
             *
             * This takes a user-supplied brain key and normalizes it into the form used
             * for generating private keys.  In particular, this upper-cases all ASCII characters
             * and collapses multiple spaces into one.
             * @param s the brain key as supplied by the user
             * @returns the brain key in its normalized form
             */
            std::string normalize_brain_key(std::string s) const;

            /**
             *  This method will genrate new owner, active, and memo keys for the new account which
             *  will be controlable by this wallet. There is a fee associated with account creation
             *  that is paid by the creator. The current account creation fee can be found with the
             *  'info' wallet command.
             *
             *  @param creator The account creating the new account
             *  @param new_account_name The name of the new account
             *  @param json_meta JSON Metadata associated with the new account
             *  @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction create_account(std::string creator, std::string new_account_name, std::string json_meta,
                                                        bool broadcast);

            /**
             * This method is used by faucets to create new accounts for other users which must
             * provide their desired keys. The resulting account may not be controllable by this
             * wallet. There is a fee associated with account creation that is paid by the creator.
             * The current account creation fee can be found with the 'info' wallet command.
             *
             * @param creator The account creating the new account
             * @param newname The name of the new account
             * @param json_meta JSON Metadata associated with the new account
             * @param owner public owner key of the new account
             * @param active public active key of the new account
             * @param posting public posting key of the new account
             * @param memo public memo key of the new account
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction create_account_with_keys(std::string creator, std::string newname, std::string json_meta,
                                                                  public_key_type owner, public_key_type active,
                                                                  public_key_type posting, public_key_type memo,
                                                                  bool broadcast) const;

            /**
             *  This method will genrate new owner, active, and memo keys for the new account which
             *  will be controlable by this wallet. There is a fee associated with account creation
             *  that is paid by the creator. The current account creation fee can be found with the
             *  'info' wallet command.
             *
             *  These accounts are created with combination of steemit and delegated SP
             *
             *  @param creator The account creating the new account
             *  @param steemit_fee The amount of the fee to be paid with steemit
             *  @param delegated_vests The amount of the fee to be paid with delegation
             *  @param new_account_name The name of the new account
             *  @param json_meta JSON Metadata associated with the new account
             *  @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            create_account_delegated(string
            creator,
            asset<0, 17, 0> steem_fee, asset<0, 17, 0>
            delegated_vests,
            string new_account_name,
            =======

            annotated_signed_transaction create_account_delegated(string creator, asset steemit_fee,
                                                                  asset delegated_vests, string new_account_name,

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
            string json_meta,
            bool broadcast
            );
=======
            annotated_signed_transaction create_account_delegated(std::string creator, asset<0, 17, 0> steem_fee,
                                                                  asset<0, 17, 0> delegated_vests, std::string new_account_name,
                                                                  std::string json_meta, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * This method is used by faucets to create new accounts for other users which must
             * provide their desired keys. The resulting account may not be controllable by this
             * wallet. There is a fee associated with account creation that is paid by the creator.
             * The current account creation fee can be found with the 'info' wallet command.
             *
             * These accounts are created with combination of steemit and delegated SP
             *
             * @param creator The account creating the new account
             * @param steemit_fee The amount of the fee to be paid with steemit
             * @param delegated_vests The amount of the fee to be paid with delegation
             * @param newname The name of the new account
             * @param json_meta JSON Metadata associated with the new account
             * @param owner public owner key of the new account
             * @param active public active key of the new account
             * @param posting public posting key of the new account
             * @param memo public memo key of the new account
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            create_account_with_keys_delegated(string
            creator,
            asset<0, 17, 0> steem_fee, asset<0, 17, 0>
            delegated_vests,
            string newname,
            =======

            annotated_signed_transaction create_account_with_keys_delegated(string creator, asset steemit_fee,
                                                                            asset delegated_vests, string newname,

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
            string json_meta, public_key_type
            owner,
            public_key_type active, public_key_type
            posting,
            public_key_type memo,
            bool broadcast
            ) const;
=======
            annotated_signed_transaction create_account_with_keys_delegated(std::string creator, asset<0, 17, 0> steem_fee,
                                                                            asset<0, 17, 0> delegated_vests, std::string newname,
                                                                            std::string json_meta, public_key_type owner,
                                                                            public_key_type active,
                                                                            public_key_type posting,
                                                                            public_key_type memo, bool broadcast) const;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * This method updates the keys of an existing account.
             *
             * @param accountname The name of the account
             * @param json_meta New JSON Metadata to be associated with the account
             * @param owner New public owner key for the account
             * @param active New public active key for the account
             * @param posting New public posting key for the account
             * @param memo New public memo key for the account
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction update_account(std::string accountname, std::string json_meta, public_key_type owner,
                                                        public_key_type active, public_key_type posting,
                                                        public_key_type memo, bool broadcast) const;

            /**
             * This method updates the key of an authority for an exisiting account.
             * Warning: You can create impossible authorities using this method. The method
             * will fail if you create an impossible owner authority, but will allow impossible
             * active and posting authorities.
             *
             * @param account_name The name of the account whose authority you wish to update
             * @param type The authority type. e.g. owner, active, or posting
             * @param key The public key to add to the authority
             * @param weight The weight the key should have in the authority. A weight of 0 indicates the removal of the key.
             * @param broadcast true if you wish to broadcast the transaction.
             */
            annotated_signed_transaction update_account_auth_key(std::string account_name, authority_type type,
                                                                 public_key_type key, weight_type weight,
                                                                 bool broadcast);

            /**
             * This method updates the account of an authority for an exisiting account.
             * Warning: You can create impossible authorities using this method. The method
             * will fail if you create an impossible owner authority, but will allow impossible
             * active and posting authorities.
             *
             * @param account_name The name of the account whose authority you wish to update
             * @param type The authority type. e.g. owner, active, or posting
             * @param auth_account The account to add the the authority
             * @param weight The weight the account should have in the authority. A weight of 0 indicates the removal of the account.
             * @param broadcast true if you wish to broadcast the transaction.
             */
            annotated_signed_transaction update_account_auth_account(std::string account_name, authority_type type,
                                                                     std::string auth_account, weight_type weight,
                                                                     bool broadcast);

            /**
             * This method updates the weight threshold of an authority for an account.
             * Warning: You can create impossible authorities using this method as well
             * as implicitly met authorities. The method will fail if you create an implicitly
             * true authority and if you create an impossible owner authoroty, but will allow
             * impossible active and posting authorities.
             *
             * @param account_name The name of the account whose authority you wish to update
             * @param type The authority type. e.g. owner, active, or posting
             * @param threshold The weight threshold required for the authority to be met
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction update_account_auth_threshold(std::string account_name, authority_type type,
                                                                       uint32_t threshold, bool broadcast);

            /**
             * This method updates the account JSON metadata
             *
             * @param account_name The name of the account you wish to update
             * @param json_meta The new JSON metadata for the account. This overrides existing metadata
             * @param broadcast ture if you wish to broadcast the transaction
             */
            annotated_signed_transaction update_account_meta(std::string account_name, std::string json_meta, bool broadcast);

            /**
             * This method updates the memo key of an account
             *
             * @param account_name The name of the account you wish to update
             * @param key The new memo public key
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction update_account_memo_key(std::string account_name, public_key_type key,
                                                                 bool broadcast);

            <<<<<<< HEAD
            =======

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

            /**
             * This method delegates VESTS from one account to another.
             *
             * @param delegator The name of the account delegating VESTS
             * @param delegatee The name of the account receiving VESTS
             * @param vesting_shares The amount of VESTS to delegate
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            delegate_vesting_shares(string
            delegator,
            string delegatee, asset<0, 17, 0>
            vesting_shares,
            bool broadcast
            );
            =======

            annotated_signed_transaction delegate_vesting_shares(string delegator, string delegatee,
                                                                 asset vesting_shares, bool broadcast);

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
=======
            annotated_signed_transaction delegate_vesting_shares(std::string delegator, std::string delegatee, asset<0, 17, 0> vesting_shares, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             *  This method is used to convert a JSON transaction to its transaction ID.
             */
            transaction_id_type get_transaction_id(const signed_transaction &trx) const {
                return trx.id();
            }

            /** Lists all witnesses registered in the blockchain.
             * This returns a list of all account names that own witnesses, and the associated witness id,
             * sorted by name.  This lists witnesses whether they are currently voted in or not.
             *
             * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all witnesss,
             * start by setting \c lowerbound to the empty std::string \c "", and then each iteration, pass
             * the last witness name returned as the \c lowerbound for the next \c list_witnesss() call.
             *
             * @param lowerbound the name of the first witness to return.  If the named witness does not exist,
             *                   the list will start at the witness that comes after \c lowerbound
             * @param limit the maximum number of witnesss to return (max: 1000)
             * @returns a list of witnesss mapping witness names to witness ids
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            vector<account_name_type> list_witnesses(const string &lowerbound, uint32_t limit);
=======
            std::set<account_name_type> list_witnesses(const std::string &lowerbound, uint32_t limit);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Returns information about the given witness.
             * @param owner_account the name or id of the witness account owner, or the id of the witness
             * @returns the information about the witness stored in the block chain
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            optional<api::witness_api_object> get_witness(string owner_account);
=======
            optional<witness_api_obj> get_witness(std::string owner_account);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Returns conversion requests by an account
             *
             * @param owner Account name of the account owning the requests
             *
             * @returns All pending conversion requests by account
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD vector<convert_request_object>
            get_conversion_requests(string
            owner);
            =======

            vector <api::convert_request_api_object> get_conversion_requests(string owner);

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
=======
            std::vector<convert_request_object> get_conversion_requests(std::string owner);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp


            /**
             * Update a witness object owned by the given account.
             *
             * @param witness_name The name of the witness account.
             * @param url A URL containing some information about the witness.  The empty std::string makes it remain the same.
             * @param block_signing_key The new block signing public key.  The empty std::string disables block production.
             * @param props The chain properties the witness is voting on.
             * @param broadcast true if you wish to broadcast the transaction.
             */
            annotated_signed_transaction update_witness(std::string witness_name, std::string url,
                                                        public_key_type block_signing_key,

            <<<<<<<
            HEAD const chain_properties<0, 17, 0>
            &props,
            bool broadcast = false
            );
            =======
            const chain_properties &props,
            bool broadcast = false
            );
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

            /** Set the voting proxy for an account.
             *
             * If a user does not wish to take an active part in voting, they can choose
             * to allow another account to vote their stake.
             *
             * Setting a vote proxy does not remove your previous votes from the blockchain,
             * they remain there but are ignored.  If you later null out your vote proxy,
             * your previous votes will take effect again.
             *
             * This setting can be changed at any time.
             *
             * @param account_to_modify the name or id of the account to update
             * @param proxy the name of account that should proxy to, or empty std::string to have no proxy
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction set_voting_proxy(std::string account_to_modify, std::string proxy,
                                                          bool broadcast = false);

            /**
             * Vote for a witness to become a block producer. By default an account has not voted
             * positively or negatively for a witness. The account can either vote for with positively
             * votes or against with negative votes. The vote will remain until updated with another
             * vote. Vote strength is determined by the accounts vesting shares.
             *
             * @param account_to_vote_with The account voting for a witness
             * @param witness_to_vote_for The witness that is being voted for
             * @param approve true if the account is voting for the account to be able to be a block produce
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction vote_for_witness(std::string account_to_vote_with, std::string witness_to_vote_for,
                                                          bool approve = true, bool broadcast = false);

            /**
             * Transfer funds from one account to another. steemit and SBD can be transferred.
             *
             * @param from The account the funds are coming from
             * @param to The account the funds are going to
             * @param amount The funds being transferred. i.e. "100.000 steemit"
             * @param memo A memo for the transactionm, encrypted with the to account's public memo key
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            transfer(string
            from,
            string to, asset<0, 17, 0>
            amount,
            string memo,
            =======

            annotated_signed_transaction transfer(string from, string to, asset amount, string memo,

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
            bool broadcast = false
            );
=======
            annotated_signed_transaction transfer(std::string from, std::string to, asset<0, 17, 0> amount, std::string memo,
                                                  bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Transfer funds from one account to another using escrow. steemit and SBD can be transferred.
             *
             * @param from The account the funds are coming from
             * @param to The account the funds are going to
             * @param agent The account acting as the agent in case of dispute
             * @param escrow_id A unique id for the escrow transfer. (from, escrow_id) must be a unique pair
             * @param sbd_amount The amount of SBD to transfer
             * @param steemit_amount The amount of steemit to transfer
             * @param fee The fee paid to the agent
             * @param ratification_deadline The deadline for 'to' and 'agent' to approve the escrow transfer
             * @param escrow_expiration The expiration of the escrow transfer, after which either party can claim the funds
             * @param json_meta JSON encoded meta data
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            annotated_signed_transaction escrow_transfer(string from, string to, string agent, uint32_t escrow_id,

            <<<<<<<
            HEAD asset<0, 17, 0>
            sbd_amount,
            asset<0, 17, 0> steem_amount, asset<0, 17, 0>
            fee,
            =======
            asset sbd_amount, asset
            steemit_amount,
            asset fee,
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
            time_point_sec ratification_deadline, time_point_sec
            escrow_expiration,
            string json_meta,
            bool broadcast = false
            );
=======
            annotated_signed_transaction escrow_transfer(std::string from, std::string to, std::string agent, uint32_t escrow_id,
                                                         asset<0, 17, 0> sbd_amount, asset<0, 17, 0> steem_amount, asset<0, 17, 0> fee,
                                                         time_point_sec ratification_deadline,
                                                         time_point_sec escrow_expiration, std::string json_meta,
                                                         bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Approve a proposed escrow transfer. Funds cannot be released until after approval. This is in lieu of requiring
             * multi-sig on escrow_transfer
             *
             * @param from The account that funded the escrow
             * @param to The destination of the escrow
             * @param agent The account acting as the agent in case of dispute
             * @param who The account approving the escrow transfer (either 'to' or 'agent)
             * @param escrow_id A unique id for the escrow transfer
             * @param approve true to approve the escrow transfer, otherwise cancels it and refunds 'from'
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction escrow_approve(std::string from, std::string to, std::string agent, std::string who,
                                                        uint32_t escrow_id, bool approve, bool broadcast = false);

            /**
             * Raise a dispute on the escrow transfer before it expires
             *
             * @param from The account that funded the escrow
             * @param to The destination of the escrow
             * @param agent The account acting as the agent in case of dispute
             * @param who The account raising the dispute (either 'from' or 'to')
             * @param escrow_id A unique id for the escrow transfer
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction escrow_dispute(std::string from, std::string to, std::string agent, std::string who,
                                                        uint32_t escrow_id, bool broadcast = false);

            /**
             * Release funds help in escrow
             *
             * @param from The account that funded the escrow
             * @param to The account the funds are originally going to
             * @param agent The account acting as the agent in case of dispute
             * @param who The account authorizing the release
             * @param receiver The account that will receive funds being released
             * @param escrow_id A unique id for the escrow transfer
             * @param sbd_amount The amount of SBD that will be released
             * @param steemit_amount The amount of steemit that will be released
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            annotated_signed_transaction escrow_release(string from, string to, string agent, string who,

            <<<<<<<
            HEAD string
            receiver,
            uint32_t escrow_id, asset<0, 17, 0>
            sbd_amount,
            asset<0, 17, 0> steem_amount,
            bool broadcast = false
            );
            =======
            string receiver, uint32_t
            escrow_id,
            asset sbd_amount, asset
            steemit_amount,
            bool broadcast = false
            );
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
=======
            annotated_signed_transaction escrow_release(std::string from, std::string to, std::string agent, std::string who,
                                                        std::string receiver, uint32_t escrow_id, asset<0, 17, 0> sbd_amount, asset<0, 17, 0> steem_amount, bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Transfer steemit into a vesting fund represented by vesting shares (VESTS). VESTS are required to vesting
             * for a minimum of one coin year and can be withdrawn once a week over a two year withdraw period.
             * VESTS are protected against dilution up until 90% of steemit is vesting.
             *
             * @param from The account the steemit is coming from
             * @param to The account getting the VESTS
             * @param amount The amount of steemit to vest i.e. "100.00 steemit"
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            transfer_to_vesting(string
            from,
            string to, asset<0, 17, 0>
            amount,
            =======

            annotated_signed_transaction transfer_to_vesting(string from, string to, asset amount,

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
            bool broadcast = false
            );
=======
            annotated_signed_transaction transfer_to_vesting(std::string from, std::string to, asset<0, 17, 0> amount,
                                                             bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             *  Transfers into savings happen immediately, transfers from savings take 72 hours
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            transfer_to_savings(string
            from,
            string to, asset<0, 17, 0>
            amount,
            string memo,
            =======

            annotated_signed_transaction transfer_to_savings(string from, string to, asset amount, string memo,

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
            bool broadcast = false
            );
=======
            annotated_signed_transaction transfer_to_savings(std::string from, std::string to, asset<0, 17, 0> amount, std::string memo,
                                                             bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * @param request_id - an unique ID assigned by from account, the id is used to cancel the operation and can be reused after the transfer completes
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            annotated_signed_transaction transfer_from_savings(string from, uint32_t request_id, string to,

            <<<<<<<
            HEAD asset<0, 17, 0>
            amount,
            string memo,
            bool broadcast = false
            );
            =======
            asset amount, string
            memo,
            bool broadcast = false
            );
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
=======
            annotated_signed_transaction transfer_from_savings(std::string from, uint32_t request_id, std::string to,
                                                               asset<0, 17, 0> amount, std::string memo, bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             *  @param request_id the id used in transfer_from_savings
             *  @param from the account that initiated the transfer
             */
            annotated_signed_transaction cancel_transfer_from_savings(std::string from, uint32_t request_id,
                                                                      bool broadcast = false);


            /**
             * Set up a vesting withdraw request. The request is fulfilled once a week over the next two year (104 weeks).
             *
             * @param from The account the VESTS are withdrawn from
             * @param vesting_shares The amount of VESTS to withdraw over the next two years. Each week (amount/104) shares are
             *    withdrawn and deposited back as steemit. i.e. "10.000000 VESTS"
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            annotated_signed_transaction withdraw_vesting(string from, asset<0, 17, 0> vesting_shares,
                                                          bool broadcast = false);
=======
            annotated_signed_transaction withdraw_vesting(std::string from, asset<0, 17, 0> vesting_shares, bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Set up a vesting withdraw route. When vesting shares are withdrawn, they will be routed to these accounts
             * based on the specified weights.
             *
             * @param from The account the VESTS are withdrawn from.
             * @param to   The account receiving either VESTS or steemit.
             * @param percent The percent of the withdraw to go to the 'to' account. This is denoted in hundreths of a percent.
             *    i.e. 100 is 1% and 10000 is 100%. This value must be between 1 and 100000
             * @param auto_vest Set to true if the from account should receive the VESTS as VESTS, or false if it should receive
             *    them as steemit.
             * @param broadcast true if you wish to broadcast the transaction.
             */
            annotated_signed_transaction set_withdraw_vesting_route(std::string from, std::string to, uint16_t percent,
                                                                    bool auto_vest, bool broadcast = false);

            /**
             *  This method will convert SBD to steemit at the current_median_history price one
             *  week from the time it is executed. This method depends upon there being a valid price feed.
             *
             *  @param from The account requesting conversion of its SBD i.e. "1.000 SBD"
             *  @param amount The amount of SBD to convert
             *  @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction convert_sbd(std::string from, asset<0, 17, 0> amount, bool broadcast = false);

            /**
             * A witness can public a price feed for the steemit:SBD market. The median price feed is used
             * to process conversion requests from SBD to steemit.
             *
             * @param witness The witness publishing the price feed
             * @param exchange_rate The desired exchange rate
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction publish_feed(std::string witness, price<0, 17, 0> exchange_rate, bool broadcast);

            /** Signs a transaction.
             *
             * Given a fully-formed transaction that is only lacking signatures, this signs
             * the transaction with the necessary keys and optionally broadcasts the transaction
             * @param tx the unsigned transaction
             * @param broadcast true if you wish to broadcast the transaction
             * @return the signed version of the transaction
             */
            annotated_signed_transaction sign_transaction(signed_transaction tx, bool broadcast = false);

            /** Returns an uninitialized object representing a given blockchain operation.
             *
             * This returns a default-initialized object of the given type; it can be used
             * during early development of the wallet when we don't yet have custom commands for
             * creating all of the operations the blockchain supports.
             *
             * Any operation the blockchain supports can be created using the transaction builder's
             * \c add_operation_to_builder_transaction() , but to do that from the CLI you need to
             * know what the JSON form of the operation looks like.  This will give you a template
             * you can fill in.  It's better than nothing.
             *
             * @param operation_type the type of operation to return, must be one of the
             *                       operations defined in `steemit/chain/operations.hpp`
             *                       (e.g., "global_parameters_update_operation")
             * @return a default-constructed operation of the given type
             */
            operation get_prototype_operation(std::string operation_type);

<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
=======
            void network_add_nodes(const std::vector<std::string> &nodes);

            std::vector<variant> network_get_connected_peers();

>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp
            /**
             * Gets the current order book for steemit:SBD
             *
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
             * @param limit Maximum number of orders to return for bids and asks. Max is 1000.
             */
            <<<<<<<
            HEAD
                    market_history::order_book

            get_order_book(const string &base, const string &quote, unsigned limit = 50);

            vector <extended_limit_order> get_limit_orders_by_owner(string account_name);

            vector <call_order_object> get_call_orders_by_owner(string account_name);

            vector <force_settlement_object> get_settle_orders_by_owner(string account_name);
=======
             * @param base Base symbol std::string
             * @param quote Quote symbol std::string
             * @param limit Maximum number of orders to return for bids and asks. Max is 1000.
             */
            market_history::order_book get_order_book(const std::string &base, const std::string &quote, unsigned limit = 50);

            std::vector<extended_limit_order> get_limit_orders_by_owner(std::string account_name);

            std::vector<call_order_object> get_call_orders_by_owner(std::string account_name);

            std::vector<force_settlement_object> get_settle_orders_by_owner(std::string account_name);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Returns the collateral_bid object for the given MPA
             *
             * @param asset the name or id of the asset
             * @param limit the number of entries to return
             * @param start the sequence number where to start looping back throw the history
             * @returns a list of \c collateral_bid_objects
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            vector <collateral_bid_object> get_collateral_bids(string asset, uint32_t limit = 100,
                                                               uint32_t start = 0) const;
=======
            std::vector<collateral_bid_object> get_collateral_bids(std::string asset, uint32_t limit = 100,
                                                              uint32_t start = 0) const;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Creates or updates a bid on an MPA after global settlement.
             *
             * In order to revive a market-pegged asset after global settlement (aka
             * black swan), investors can bid collateral in order to take over part of
             * the debt and the settlement fund, see BSIP-0018. Updating an existing
             * bid to cover 0 debt will delete the bid.
             *
             * @param bidder_name the name or id of the account making the bid
             * @param debt_amount the amount of debt of the named asset to bid for
             * @param debt_symbol the name or id of the MPA to bid for
             * @param additional_collateral the amount of additional collateral to bid
             *        for taking over debt_amount. The asset type of this amount is
             *        determined automatically from debt_symbol.
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction creating/updating the bid
             */

            signed_transaction bid_collateral(std::string bidder_name, std::string debt_amount, std::string debt_symbol,
                                              std::string additional_collateral, bool broadcast = false);

            =======
            // TODO
            // market_history::get_order_book_return get_order_book( uint32_t limit = 1000 );
            // TODO
            // vector< condenser_api::extended_limit_order > get_open_orders( string accountname );
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

            /**
             *  Creates a limit order at the price amount_to_sell / min_to_receive and will deduct amount_to_sell from account
             *
             *  @param owner The name of the account creating the order
             *  @param order_id is a unique identifier assigned by the creator of the order, it can be reused after the order has been filled
             *  @param amount_to_sell The amount of either SBD or steemit you wish to sell
             *  @param min_to_receive The amount of the other asset you will receive at a minimum
             *  @param fill_or_kill true if you want the order to be killed if it cannot immediately be filled
             *  @param expiration the time the order should expire if it has not been filled
             *  @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            create_order(string
            owner,
            uint32_t order_id, asset<0, 17, 0>
            amount_to_sell,
            asset<0, 17, 0> min_to_receive,
            bool fill_or_kill, uint32_t
            expiration,
            =======

            annotated_signed_transaction create_order(string owner, uint32_t order_id, asset amount_to_sell,
                                                      asset min_to_receive, bool fill_or_kill, uint32_t expiration,

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
            bool broadcast
            );
=======
            annotated_signed_transaction create_order(std::string owner, uint32_t order_id, asset<0, 17, 0> amount_to_sell,
                                                      asset<0, 17, 0> min_to_receive, bool fill_or_kill, uint32_t expiration,
                                                      bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Cancel an order created with create_order
             *
             * @param owner The name of the account owning the order to cancel_order
             * @param orderid The unique identifier assigned to the order by its creator
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            <<<<<<<
            HEAD annotated_signed_transaction
            cancel_order(string
            owner,
            protocol::integral_id_type order_id,
            bool broadcast
            );
=======
            annotated_signed_transaction cancel_order(std::string owner, protocol::integral_id_type order_id, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Place a limit order attempting to sell one asset for another.
             *
             * Buying and selling are the same operation on Graphene; if you want to buy GOLOS
             * with USD, you should sell USD for GOLOS.
             *
             * The blockchain will attempt to sell the \c symbol_to_sell for as
             * much \c symbol_to_receive as possible, as long as the price is at
             * least \c min_to_receive / \c amount_to_sell.
             *
             * In addition to the transaction fees, market fees will apply as specified
             * by the issuer of both the selling asset and the receiving asset as
             * a percentage of the amount exchanged.
             *
             * If either the selling asset or the receiving asset is whitelist
             * restricted, the order will only be created if the seller is on
             * the whitelist of the restricted asset type.
             *
             * Market orders are matched in the order they are included
             * in the block chain.
             *
             * @todo Allow order expiration to be set here.  Document default/max expiration time
             *
             * @param seller_account the account providing the asset being sold, and which will
             *                       receive the proceeds of the sale.
             * @param amount_to_sell the amount of the asset being sold to sell (in nominal units)
             * @param amount_to_receive the minimum amount you are willing to receive in return for
             *                       selling the entire amount_to_sell
             * @param timeout_sec if the order does not fill immediately, this is the length of
             *                    time the order will remain on the order books before it is
             *                    cancelled and the un-spent funds are returned to the seller's
             *                    account
             * @param fill_or_kill if true, the order will only be included in the blockchain
             *                     if it is filled immediately; if false, an open order will be
             *                     left on the books to fill any amount that cannot be filled
             *                     immediately.
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction selling the funds
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction sell_asset(string seller_account, asset<0, 17, 0> amount_to_sell,
                                          asset<0, 17, 0> amount_to_receive, uint32_t timeout_sec,
                                          protocol::integral_id_type order_id, bool fill_or_kill, bool broadcast);
=======
            signed_transaction sell_asset(std::string seller_account, asset<0, 17, 0> amount_to_sell, asset<0, 17, 0> amount_to_receive, uint32_t timeout_sec, protocol::integral_id_type order_id, bool fill_or_kill, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Place a limit order attempting to sell one asset for another.
             *
             * This API call abstracts away some of the details of the sell_asset call to be more
             * user friendly. All orders placed with sell never timeout and will not be killed if they
             * cannot be filled immediately. If you wish for one of these parameters to be different,
             * then sell_asset should be used instead.
             *
             * @param seller_account the account providing the asset being sold, and which will
             *                       receive the processed of the sale.
             * @param base The name or id of the asset to sell.
             * @param quote The name or id of the asset to recieve.
             * @param rate The rate in base:quote at which you want to sell.
             * @param amount The amount of base you want to sell.
             * @param broadcast true to broadcast the transaction on the network.
             * @returns The signed transaction selling the funds.
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction sell(string seller_account, string base, string quote, double rate, double amount,
                                    protocol::integral_id_type order_id, bool broadcast);
=======
            signed_transaction sell(std::string seller_account, asset<0, 17, 0> base, asset<0, 17, 0> quote, protocol::integral_id_type order_id, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Place a limit order attempting to buy one asset with another.
             *
             * This API call abstracts away some of the details of the sell_asset call to be more
             * user friendly. All orders placed with buy never timeout and will not be killed if they
             * cannot be filled immediately. If you wish for one of these parameters to be different,
             * then sell_asset should be used instead.
             *
             * @param buyer_account The account buying the asset for another asset.
             * @param base The name or id of the asset to buy.
             * @param quote The name or id of the asset being offered as payment.
             * @param rate The rate in base:quote at which you want to buy.
             * @param amount the amount of base you want to buy.
             * @param broadcast true to broadcast the transaction on the network.
             * @returns The signed transaction selling the funds.
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction buy(string buyer_account, string base, string quote, double rate, double amount,
                                   protocol::integral_id_type order_id, bool broadcast);
=======
            signed_transaction buy(std::string buyer_account, asset<0, 17, 0> base, asset<0, 17, 0> quote, protocol::integral_id_type order_id, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Borrow an asset or update the debt/collateral ratio for the loan.
             *
             * This is the first step in shorting an asset.  Call \c sell_asset() to complete the short.
             * @param borrower_name the name or id of the account associated with the transaction.
             * @param amount_to_borrow the amount of the asset being borrowed.
             * Make this value negative to pay back debt.
             * @param amount_of_collateral the amount of the backing asset to add to your collateral
             *        position.  Make this negative to claim back some of your collateral.
             *        The backing asset is defined in the \c bitasset_options for the asset being borrowed.
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction borrowing the asset
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction borrow_asset(string borrower_name, asset<0, 17, 0> amount_to_borrow,
                                            asset<0, 17, 0> amount_of_collateral, bool broadcast);
=======
            signed_transaction borrow_asset(std::string borrower_name, asset<0, 17, 0> amount_to_borrow, asset<0, 17, 0> amount_of_collateral,
                                            bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Creates a new user-issued or market-issued asset.
             *
             * Many options can be changed later using \c update_asset()
             *
             * Right now this function is difficult to use because you must provide raw JSON data
             * structures for the options objects, and those include prices and asset ids.
             *
             * @param issuer the name or id of the account who will pay the fee and become the
             *               issuer of the new asset.  This can be updated later
             * @param symbol the ticker symbol of the new asset
             * @param precision the number of digits of precision to the right of the decimal point,
             *                  must be less than or equal to 12
             * @param common asset options required for all new assets.
             *               Note that core_exchange_rate technically needs to store the asset ID of
             *               this new asset. Since this ID is not known at the time this operation is
             *               created, create this price as though the new asset has instance ID 1, and
             *               the chain will overwrite it with the new asset's ID.
             * @param bitasset_opts options specific to BitAssets.  This may be null unless the
             *               \c market_issued flag is set in common.flags
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction creating a new asset
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction create_asset(string issuer, string symbol, uint8_t precision,
                                            asset_options<0, 17, 0> common,
=======
            signed_transaction create_asset(std::string issuer, std::string symbol, uint8_t precision, asset_options<0, 17, 0> common,
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp
                                            fc::optional<bitasset_options> bitasset_opts, bool broadcast = false);

            /** Issue new shares of an asset.
             *
             * @param to_account the name or id of the account to receive the new shares
             * @param amount the amount to issue, in nominal units
             * @param memo a memo to include in the transaction, readable by the recipient
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction issuing the new shares
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction issue_asset(string to_account, asset<0, 17, 0> amount, string memo,
                                           bool broadcast = false);
=======
            signed_transaction issue_asset(std::string to_account, asset<0, 17, 0> amount, std::string memo, bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Update the core options on an asset.
             * There are a number of options which all assets in the network use. These options are
             * enumerated in the asset_object::asset_options<0, 17, 0> struct. This command is used to update
             * these options for an existing asset.
             *
             * @note This operation cannot be used to update BitAsset-specific options. For these options,
             * \c update_bitasset() instead.
             *
             * @param symbol the name or id of the asset to update
             * @param new_issuer if changing the asset's issuer, the name or id of the new issuer.
             *                   null if you wish to remain the issuer of the asset
             * @param new_options the new asset_options<0, 17, 0> object, which will entirely replace the existing
             *                    options.
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction updating the asset
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction update_asset(string symbol, optional<string> new_issuer,
                                            asset_options<0, 17, 0> new_options, bool broadcast = false);
=======
            signed_transaction update_asset(std::string symbol, optional<std::string> new_issuer, asset_options<0, 17, 0> new_options,
                                            bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /** Update the options specific to a BitAsset.
             *
             * BitAssets have some options which are not relevant to other asset types. This operation is used to update those
             * options an an existing BitAsset.
             *
             * @see update_asset()
             *
             * @param symbol the name or id of the asset to update, which must be a market-issued asset
             * @param new_options the new bitasset_options object, which will entirely replace the existing
             *                    options.
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction updating the bitasset
             */
            signed_transaction update_bitasset(std::string symbol, bitasset_options new_options, bool broadcast = false);

            /** Update the set of feed-producing accounts for a BitAsset.
             *
             * BitAssets have price feeds selected by taking the median values of recommendations from a set of feed producers.
             * This command is used to specify which accounts may produce feeds for a given BitAsset.
             * @param symbol the name or id of the asset to update
             * @param new_feed_producers a list of account names or ids which are authorized to produce feeds for the asset.
             *                           this list will completely replace the existing list
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction updating the bitasset's feed producers
             */
            signed_transaction update_asset_feed_producers(std::string symbol, flat_set<std::string> new_feed_producers,
                                                           bool broadcast = false);

            /** Publishes a price feed for the named asset.
             *
             * Price feed providers use this command to publish their price feeds for market-issued assets. A price feed is
             * used to tune the market for a particular market-issued asset. For each value in the feed, the median across all
             * committee_member feeds for that asset is calculated and the market for the asset is configured with the median of that
             * value.
             *
             * The feed object in this command contains three prices: a call price limit, a short price limit, and a settlement price.
             * The call limit price is structured as (collateral asset) / (debt asset) and the short limit price is structured
             * as (asset for sale) / (collateral asset). Note that the asset IDs are opposite to eachother, so if we're
             * publishing a feed for USD, the call limit price will be CORE/USD and the short limit price will be USD/CORE. The
             * settlement price may be flipped either direction, as long as it is a ratio between the market-issued asset and
             * its collateral.
             *
             * @param publishing_account the account publishing the price feed
             * @param symbol the name or id of the asset whose feed we're publishing
             * @param feed the price_feed object containing the three prices making up the feed
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction updating the price feed for the given asset
             */
            signed_transaction publish_asset_feed(std::string publishing_account, std::string symbol, price_feed<0, 17, 0> feed,
                                                  bool broadcast = false);

            /** Pay into the fee pool for the given asset.
             *
             * User-issued assets can optionally have a pool of the core asset which is
             * automatically used to pay transaction fees for any transaction using that
             * asset (using the asset's core exchange rate).
             *
             * This command allows anyone to deposit the core asset into this fee pool.
             *
             * @param from the name or id of the account sending the core asset
             * @param symbol the name or id of the asset whose fee pool you wish to fund
             * @param amount the amount of the core asset to deposit
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction funding the fee pool
             */
            signed_transaction fund_asset_fee_pool(std::string from, std::string symbol, std::string amount, bool broadcast = false);

            /** Burns the given user-issued asset.
             *
             * This command burns the user-issued asset to reduce the amount in circulation.
             * @note you cannot burn market-issued assets.
             * @param from the account containing the asset you wish to burn
             * @param amount the amount to burn, in nominal units
             * @param symbol the name or id of the asset to burn
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction burning the asset
             */
            signed_transaction reserve_asset(std::string from, std::string amount, std::string symbol, bool broadcast = false);

            /** Forces a global settling of the given asset (black swan or prediction markets).
             *
             * In order to use this operation, asset_to_settle must have the global_settle flag set
             *
             * When this operation is executed all balances are converted into the backing asset at the
             * settle_price and all open margin positions are called at the settle price.  If this asset is
             * used as backing for other bitassets, those bitassets will be force settled at their current
             * feed price.
             *
             * @note this operation is used only by the asset issuer, \c settle_asset() may be used by
             *       any user owning the asset
             *
             * @param symbol the name or id of the asset to force settlement on
             * @param settle_price the price at which to settle
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction settling the named asset
             */
            signed_transaction global_settle_asset(std::string symbol, price<0, 17, 0> settle_price, bool broadcast = false);

            /** Schedules a market-issued asset for automatic settlement.
             *
             * Holders of market-issued assests may request a forced settlement for some amount of their asset. This means that
             * the specified sum will be locked by the chain and held for the settlement period, after which time the chain will
             * choose a margin posision holder and buy the settled asset using the margin's collateral. The price of this sale
             * will be based on the feed price for the market-issued asset being settled. The exact settlement price will be the
             * feed price at the time of settlement with an offset in favor of the margin position, where the offset is a
             * blockchain parameter set in the global_property_object.
             *
             * @param account_to_settle the name or id of the account owning the asset
             * @param amount_to_settle the amount of the named asset to schedule for settlement
             * @param symbol the name or id of the asset to settlement on
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction settling the named asset
             */
            signed_transaction settle_asset(std::string account_to_settle, std::string amount_to_settle, std::string symbol,
                                            bool broadcast = false);

            /** Whitelist and blacklist accounts, primarily for transacting in whitelisted assets.
             *
             * Accounts can freely specify opinions about other accounts, in the form of either whitelisting or blackl      isting
             * them. This information is used in chain validation only to determine whether an account is authorized to tra      nsact
             * in an asset type which enforces a whitelist, but third parties can use this information for other uses as wel      l,
             * as long as it does not conflict with the use of whitelisted assets.
             *
             * An asset which enforces a whitelist specifies a list of accounts to maintain its whitelist, and a list of
             * accounts to maintain its blacklist. In order for a given account A to hold and transact in a whitel      isted asset S,
             * A must be whitelisted by at least one of S's whitelist_authorities and blacklisted by none of S's
             * blacklist_authorities. If A receives a balance of S, and is later removed from the whitelist(s) which allowe      d it
             * to hold S, or added to any blacklist S specifies as authoritative, A's balance of S will be frozen until       A's
             * authorization is reinstated.
             *
             * @param authorizing_account the account who is doing the whitelisting
             * @param account_to_list the account being whitelisted
             * @param new_listing_status the new whitelisting status
             * @param broadcast true to broadcast the transaction on the network
             * @returns the signed transaction changing the whitelisting status
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            signed_transaction whitelist_account(string authorizing_account, string account_to_list,
                                                 account_whitelist_operation<0, 17,
                                                         0>::account_listing new_listing_status,
                                                 bool broadcast = false);

            =======

            annotated_signed_transaction cancel_order(string owner, uint32_t orderid, bool broadcast);

            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
=======
            signed_transaction whitelist_account(std::string authorizing_account, std::string account_to_list, account_whitelist_operation<0, 17, 0>::account_listing new_listing_status, bool broadcast = false);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             *  Post or update a comment.
             *
             *  @param author the name of the account authoring the comment
             *  @param permlink the accountwide unique permlink for the comment
             *  @param parent_author can be null if this is a top level comment
             *  @param parent_permlink becomes category if parent_author is ""
             *  @param title the title of the comment
             *  @param body the body of the comment
             *  @param json the json metadata of the comment
             *  @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction post_comment(std::string author, std::string permlink, std::string parent_author,
                                                      std::string parent_permlink, std::string title, std::string body, std::string json,
                                                      bool broadcast);

            <<<<<<<
            HEAD

            /**
             * Extend the comment payout window by passing the required SBD to spend
             *
             * @param payer the name of the account paying for the transaction
             * @param author the name of the account authoring the comment
             * @param permlink comment permlink
             * @param extension_cost SBD amount payer will spend on payout window extension
             * @param broadcast true if you wish to broadcast the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
                    annotated_signed_transaction
            extend_payout_by_cost(string
            payer,
            string author, string
            permlink,
            asset<0, 17, 0> extension_cost,
            bool broadcast
            );
=======
            annotated_signed_transaction extend_payout_by_cost(std::string payer, std::string author, std::string permlink,
                                                               asset<0, 17, 0> extension_cost, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Extend the comment payout window by passing the required SBD to spend
             *
             * @param payer the name of the account paying for the transaction
             * @param author the name of the account authoring the comment
             * @param permlink comment permlink
             * @param extension_time the payout window final time point
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction extend_payout_by_time(std::string payer, std::string author, std::string permlink,
                                                               fc::time_point_sec extension_time, bool broadcast);

            /**
             * Send the encrypted private email-like message to user
             * @param from message author name
             * @param to message recipient name
             * @param subject message subject
             * @param body message content
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction send_private_message(std::string from, std::string to, std::string subject, std::string body,
                                                              bool broadcast);

            /**
             * Retrieves the private message inbox for the account mentioned
             * @param account account to retrieve inbox for
             * @param newest timestamp to start retrieve messages from
             * @param limit amount of messages to retrieve
             * @return message api objects vector
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            vector <extended_message_object> get_inbox(string account, fc::time_point newest, uint32_t limit);
=======
            std::vector<extended_message_object> get_inbox(std::string account, fc::time_point newest, uint32_t limit);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Retrieves the private message outbox for the account mentioned
             * @param account account to retrieve outbox for
             * @param newest timestamp to start retireve messages from
             * @param limit amount of messages to retrieve
             * @return message api objects vector
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            vector <extended_message_object> get_outbox(string account, fc::time_point newest, uint32_t limit);
=======
            std::vector<extended_message_object> get_outbox(std::string account, fc::time_point newest, uint32_t limit);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            =======
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

            /**
             * Vote on a comment to be paid steemit
             *
             * @param voter The account voting
             * @param author The author of the comment to be voted on
             * @param permlink The permlink of the comment to be voted on. (author, permlink) is a unique pair
             * @param weight The weight [-100,100] of the vote
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction vote(std::string voter, std::string author, std::string permlink, int16_t weight,
                                              bool broadcast);

            /**
             * Sets the amount of time in the future until a transaction expires.
             */
            void set_transaction_expiration(uint32_t seconds);

            /**
             * Challenge a user's authority. The challenger pays a fee to the challenged which is depositted as
             * steemit Power. Until the challenged proves their active key, all posting rights are revoked.
             *
             * @param challenger The account issuing the challenge
             * @param challenged The account being challenged
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction challenge(std::string challenger, std::string challenged, bool broadcast);

            /**
             * Create an account recovery request as a recover account. The syntax for this command contains a serialized authority object
             * so there is an example below on how to pass in the authority.
             *
             * request_account_recovery "your_account" "account_to_recover" {"weight_threshold": 1,"account_auths": [], "key_auths": [["new_public_key",1]]} true
             *
             * @param recovery_account The name of your account
             * @param account_to_recover The name of the account you are trying to recover
             * @param new_authority The new owner authority for the recovered account. This should be given to you by the holder of the compromised or lost account.
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction request_account_recovery(std::string recovery_account, std::string account_to_recover,
                                                                  authority new_authority, bool broadcast);

            /**
             * Recover your account using a recovery request created by your recovery account. The syntax for this commain contains a serialized
             * authority object, so there is an example below on how to pass in the authority.
             *
             * recover_account "your_account" {"weight_threshold": 1,"account_auths": [], "key_auths": [["old_public_key",1]]} {"weight_threshold": 1,"account_auths": [], "key_auths": [["new_public_key",1]]} true
             *
             * @param account_to_recover The name of your account
             * @param recent_authority A recent owner authority on your account
             * @param new_authority The new authority that your recovery account used in the account recover request.
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction recover_account(std::string account_to_recover, authority recent_authority,
                                                         authority new_authority, bool broadcast);

            /**
             * Change your recovery account after a 30 day delay.
             *
             * @param owner The name of your account
             * @param new_recovery_account The name of the recovery account you wish to have
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction change_recovery_account(std::string owner, std::string new_recovery_account,
                                                                 bool broadcast);

<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            vector <api::owner_authority_history_api_object> get_owner_history(string account) const;
=======
            std::vector<owner_authority_history_api_obj> get_owner_history(std::string account) const;
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

            /**
             * Prove an account's active authority, fulfilling a challenge, restoring posting rights, and making
             * the account immune to challenge for 24 hours.
             *
             * @param challenged The account that was challenged and is proving its authority.
             * @param broadcast true if you wish to broadcast the transaction
             */
            annotated_signed_transaction prove(std::string challenged, bool broadcast);

            /**
             *  Account operations have sequence numbers from 0 to N where N is the most recent operation. This method
             *  returns operations in the range [from-limit, from]
             *
             *  @param account - account whose history will be returned
             *  @param from - the absolute sequence number, -1 means most recent, limit is the number of operations before from.
             *  @param limit - the maximum number of items that can be queried (0 to 1000], must be less than from
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            // map< uint32_t, account_history::api_operation_object > get_account_history( string account, uint32_t from, uint32_t limit );
=======
            std::map<uint32_t, applied_operation> get_account_history(std::string account, uint32_t from, uint32_t limit);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp


            /**
             *  Marks one account as following another account.  Requires the posting authority of the follower.
             *
             *  @param what - a set of things to follow: posts, comments, votes, ignore
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
            annotated_signed_transaction follow(string follower, string following, set <string> what, bool broadcast);
=======
            annotated_signed_transaction follow(std::string follower, std::string following, std::set<std::string> what, bool broadcast);
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp


            std::map<std::string, std::function<std::string(fc::variant, const fc::variants &)>> get_result_formatters() const;

            fc::signal<void(bool)> lock_changed;
            std::shared_ptr<detail::wallet_api_impl> my;

            void encrypt_keys();

            /**
             * Checks memos against private keys on account and imported in wallet
             */
            void check_memo(const string &memo, const api::account_api_object &account) const;

            /**
             *  Returns the encrypted memo if memo starts with '#' otherwise returns memo
             */
            std::string get_encrypted_memo(std::string from, std::string to, std::string memo);

            /**
             * Returns the decrypted memo if possible given wallet's known private keys
             */
            std::string decrypt_memo(std::string memo);

            annotated_signed_transaction decline_voting_rights(std::string account, bool decline, bool broadcast);

            <<<<<<<
            HEAD
            /** Approve or disapprove a proposal.
             *
             * @param owner The account paying the fee for the op.
             * @param proposal_id The proposal to modify.
             * @param delta Members contain approvals to create or remove. In JSON you can leave empty members undefi      ned.
             * @param broadcast true if you wish to broadcast the transaction
             * @return the signed version of the transaction
             */
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
                    signed_transaction

            approve_proposal(const string &owner, integral_id_type proposal_id, const approval_delta &delta,
                             bool broadcast /* = false */
=======
            signed_transaction approve_proposal(const std::string &owner, integral_id_type proposal_id,
                                                const approval_delta &delta, bool broadcast /* = false */
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp
            );

            /**
             * @ingroup Transaction Builder API
             */
            transaction_handle_type begin_builder_transaction();

            /**
             * @ingroup Transaction Builder API
             */
            void add_operation_to_builder_transaction(transaction_handle_type transaction_handle, const operation &op);

            /**
             * @ingroup Transaction Builder API
             */
            void replace_operation_in_builder_transaction(transaction_handle_type handle, unsigned operation_index,
                                                          const operation &new_op);

            /**
             * @ingroup Transaction Builder API
             */
            transaction preview_builder_transaction(transaction_handle_type handle);

            /**
             * @ingroup Transaction Builder API
             */
            signed_transaction sign_builder_transaction(transaction_handle_type transaction_handle,
                                                        bool broadcast = true);

            /**
             * @ingroup Transaction Builder API
             */
            signed_transaction propose_builder_transaction(transaction_handle_type handle,
                                                           time_point_sec expiration = time_point::now() +
                                                                                       fc::minutes(1),
                                                           uint32_t review_period_seconds = 0, bool broadcast = true);

            signed_transaction propose_builder_transaction2(transaction_handle_type handle, std::string account_name_or_id,
                                                            time_point_sec expiration = time_point::now() +
                                                                                        fc::minutes(1),
                                                            uint32_t review_period_seconds = 0, bool broadcast = true);

            /**
             * @ingroup Transaction Builder API
             */
            void remove_builder_transaction(transaction_handle_type handle);

            =======
            // annotated_signed_transaction claim_reward_balance( string account, asset reward_steemit, asset reward_sbd, asset reward_vests, bool broadcast );
            >>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
        };

        struct plain_keys {
            fc::sha512 checksum;
            std::map<public_key_type, std::string> keys;
        };

    }
}

<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
<<<<<<< HEAD
=======
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp
FC_REFLECT((golos::wallet::wallet_data), (cipher_keys)(ws_server)(ws_user)(ws_password))

FC_REFLECT((golos::wallet::brain_key_info), (brain_priv_key)(wif_priv_key)(pub_key))

FC_REFLECT_DERIVED((golos::wallet::signed_block_with_info), ((golos::chain::signed_block)),
                   (block_id)(signing_key)(transaction_ids))

FC_REFLECT((golos::wallet::plain_keys), (checksum)(keys))
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
=======

FC_REFLECT((golos::wallet::wallet_data), (cipher_keys)(ws_server)(ws_user)(ws_password))

FC_REFLECT((golos::wallet::brain_key_info), (brain_priv_key)(wif_priv_key)(pub_key))
>>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73

FC_REFLECT((golos::wallet::plain_keys), ((checksum)(keys)))

<<<<<<< HEAD
=======

FC_REFLECT_ENUM(golos::wallet::authority_type, (owner)(active)(posting))

>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp
FC_REFLECT((golos::wallet::approval_delta),
           (active_approvals_to_add)(active_approvals_to_remove)(owner_approvals_to_add)(owner_approvals_to_remove)(
                   posting_approvals_to_add)(posting_approvals_to_remove)(key_approvals_to_add)(
                   key_approvals_to_remove))

FC_API(golos::wallet::wallet_api,
/// wallet api
       (help)(get_help)(about)(is_new)(is_locked)(lock)(unlock)(set_password)(load_wallet_file)(save_wallet_file)

               /// key api
               (import_key)(suggest_brain_key)(list_keys)(get_private_key)(get_private_key_from_password)(
               normalize_brain_key)

               /// query api
               (info)(list_my_accounts)(list_accounts)(list_witnesses)(get_witness)(get_steem_per_mvests)(
               get_account_count)(get_account)(get_block)(get_ops_in_block)(get_feed_history)(get_conversion_requests)(
               get_account_history)(get_withdraw_routes)

               /// transaction api
               (create_account)(create_account_with_keys)(create_account_delegated)(create_account_with_keys_delegated)(
               update_account)(update_account_auth_key)(update_account_auth_account)(update_account_auth_threshold)(
               update_account_meta)(update_account_memo_key)(delegate_vesting_shares)(update_witness)(set_voting_proxy)(
               vote_for_witness)(follow)(transfer)(transfer_to_vesting)(withdraw_vesting)(set_withdraw_vesting_route)(
               convert_sbd)(publish_feed)(get_order_book)(get_limit_orders_by_owner)(get_call_orders_by_owner)(
               get_settle_orders_by_owner)(create_order)(cancel_order)(post_comment)(extend_payout_by_cost)(
               extend_payout_by_time)(vote)(set_transaction_expiration)(challenge)(prove)(request_account_recovery)(
               recover_account)(change_recovery_account)(get_owner_history)(transfer_to_savings)(transfer_from_savings)(
               cancel_transfer_from_savings)(get_encrypted_memo)(decrypt_memo)(decline_voting_rights)

               /// private message api
               (send_private_message)(get_inbox)(get_outbox)

               /// helper api
               (get_prototype_operation)(serialize_transaction)(sign_transaction)

               (network_add_nodes)(network_get_connected_peers)

               (get_active_witnesses)(get_miner_queue)(get_transaction)

               (list_account_balances)(list_assets)(sell_asset)(sell)(buy)(borrow_asset)

               (create_asset)(update_asset)(update_bitasset)(update_asset_feed_producers)(publish_asset_feed)(
               issue_asset)(get_asset)(get_bitasset_data)(fund_asset_fee_pool)(reserve_asset)(global_settle_asset)(
               settle_asset)(whitelist_account)(bid_collateral)(get_collateral_bids)

               (get_proposal)(approve_proposal)

               /// transaction builder api
               (begin_builder_transaction)(add_operation_to_builder_transaction)(
               replace_operation_in_builder_transaction)(preview_builder_transaction)(sign_builder_transaction)(
               propose_builder_transaction)(propose_builder_transaction2)(remove_builder_transaction))
<<<<<<< HEAD:libraries/wallet/include/golos/wallet/wallet.hpp
=======
FC_REFLECT_ENUM((golos::wallet::authority_type), ((owner)(active)(posting)))

FC_API(golos::wallet::wallet_api,
// wallet api
// (help)(gethelp)
       (about)(is_new)(is_locked)(lock)(unlock)(set_password)
// (load_wallet_file)(save_wallet_file)

// /// key api
// (import_key)
// (suggest_brain_key)
// (list_keys)
// (get_private_key)
// (get_private_key_from_password)
// (normalize_brain_key)

// /// query api
// (info)
// // (list_my_accounts)
// (list_accounts)
// (list_witnesses)
// (get_witness)
// (get_account)
// // (get_block)
// // (get_ops_in_block)
// (get_feed_history)
// (get_conversion_requests)
// /// (get_account_history)
// /// (get_state)
// /// (get_withdraw_routes)

// /// transaction api
// (create_account)
// (create_account_with_keys)
// (create_account_delegated)
// (create_account_with_keys_delegated)
// (update_account)
// (update_account_auth_key)
// (update_account_auth_account)
// (update_account_auth_threshold)
// (update_account_meta)
// (update_account_memo_key)
// (delegate_vesting_shares)
// (update_witness)
// (set_voting_proxy)
// (vote_for_witness)
// (follow)
// (transfer)
// (escrow_transfer)
// (escrow_approve)
// (escrow_dispute)
// (escrow_release)
// (transfer_to_vesting)
// (withdraw_vesting)
// (set_withdraw_vesting_route)
// (convert_sbd)
// (publish_feed)
// /// (get_order_book)
// /// (get_open_orders)
// (create_order)
// (cancel_order)
// (post_comment)
// (vote)
// (set_transaction_expiration)
// (challenge)
// (prove)
// (request_account_recovery)
// (recover_account)
// (change_recovery_account)
// (get_owner_history)
// (transfer_to_savings)
// (transfer_from_savings)
// (cancel_transfer_from_savings)
// (get_encrypted_memo)
// (decrypt_memo)
// (decline_voting_rights)
// // (claim_reward_balance)

// /// helper api
// (get_prototype_operation)
// (serialize_transaction)
// (sign_transaction)

// (get_active_witnesses)
// (get_transaction)
)
>>>>>>> 09d6d4f2b7da92297d03565e85ea82bf8075ae73
=======
>>>>>>> golos-v0.17.0:libraries/wallet/include/golos/wallet/wallet.hpp

FC_REFLECT((golos::wallet::memo_data), (from)(to)(nonce)(check)(encrypted))
