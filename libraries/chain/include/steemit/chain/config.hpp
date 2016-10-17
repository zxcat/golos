/*
 * Copyright (c) 2016 Steemit Inc, Golos, community and contributors.
 */
#pragma once

#define STEEMIT_BLOCKCHAIN_VERSION              ( version(0, 14, 3) )
#define STEEMIT_BLOCKCHAIN_HARDFORK_VERSION     ( hardfork_version( STEEMIT_BLOCKCHAIN_VERSION ) )

#ifdef IS_TEST_NET
#define STEEMIT_INIT_PRIVATE_KEY                (fc::ecc::private_key::regenerate(fc::sha256::hash(std::string("init_key"))))
#define STEEMIT_INIT_PUBLIC_KEY_STR             (std::string( steemit::chain::public_key_type(STEEMIT_INIT_PRIVATE_KEY.get_public_key()) ))
#define BLOCKCHAIN_NAME                         "GOLOSTEST"
#define STEEMIT_CHAIN_ID                        (fc::sha256::hash(BLOCKCHAIN_NAME))

#define VESTS_SYMBOL  (uint64_t(6) | (uint64_t('V') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40)) ///< VESTS with 6 digits of precision
#define STEEM_SYMBOL  (uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40)) ///< STEEM with 3 digits of precision
#define SBD_SYMBOL    (uint64_t(3) | (uint64_t('T') << 8) | (uint64_t('B') << 16) | (uint64_t('D') << 24) ) ///< Test Backed Dollars with 3 digits of precision

#define STEEMIT_SYMBOL                          "TEST"
#define STEEMIT_ADDRESS_PREFIX                  "TST"

#define STEEMIT_GENESIS_TIME                    (fc::time_point_sec(1476788400))
#define STEEMIT_CASHOUT_WINDOW_SECONDS          (60*60) /// 1 hr
#define STEEMIT_SECOND_CASHOUT_WINDOW           (60*60*4) /// 4 hours
#define STEEMIT_MAX_CASHOUT_WINDOW_SECONDS      (60*60*2) /// 2 housrs
#define STEEMIT_VOTE_CHANGE_LOCKOUT_PERIOD      (60*10) /// 10 minutes

#define STEEMIT_MIN_ACCOUNT_CREATION_FEE        0

#define STEEMIT_OWNER_AUTH_RECOVERY_PERIOD                  fc::seconds(60)
#define STEEMIT_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD  fc::seconds(12)
#define STEEMIT_OWNER_UPDATE_LIMIT                          fc::seconds(0)
#define STEEMIT_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM 1
#else // IS LIVE STEEM NETWORK

#define STEEMIT_INIT_PUBLIC_KEY_STR             "GLS79mEZV96FXMdJXYgd1QHfjsWLyHhxr5zSaDP9yXbLvr8QGv5yU"
#define BLOCKCHAIN_NAME                         "GOLOS"
#define STEEMIT_CHAIN_ID                        (fc::sha256::hash(BLOCKCHAIN_NAME))

#define VESTS_SYMBOL  (uint64_t(6) | (uint64_t('G') << 8) | (uint64_t('E') << 16) | (uint64_t('S') << 24) | (uint64_t('T') << 32) | (uint64_t('S') << 40)) ///< GESTS with 6 digits of precision
#define STEEM_SYMBOL  (uint64_t(3) | (uint64_t('G') << 8) | (uint64_t('O') << 16) | (uint64_t('L') << 24) | (uint64_t('O') << 32) | (uint64_t('S') << 40)) ///< STEEM with 3 digits of precision
#define SBD_SYMBOL    (uint64_t(3) | (uint64_t('G') << 8) | (uint64_t('B') << 16) | (uint64_t('G') << 24) ) ///< STEEM Backed Dollars with 3 digits of precision
#define STEEMIT_SYMBOL                          "GOLOS"
#define STEEMIT_ADDRESS_PREFIX                  "GLS"

#define STEEMIT_GENESIS_TIME                    (fc::time_point_sec(1476722695))
#define STEEMIT_CASHOUT_WINDOW_SECONDS          (60*60*24)  /// 24 hours
#define STEEMIT_SECOND_CASHOUT_WINDOW           (60*60*24*30) /// 30 days
#define STEEMIT_MAX_CASHOUT_WINDOW_SECONDS      (60*60*24*14) /// 2 weeks
#define STEEMIT_VOTE_CHANGE_LOCKOUT_PERIOD      (60*60*2) /// 2 hours

#define STEEMIT_MIN_ACCOUNT_CREATION_FEE        1

#define STEEMIT_OWNER_AUTH_RECOVERY_PERIOD                  fc::days(30)
#define STEEMIT_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD  fc::days(1)
#define STEEMIT_OWNER_UPDATE_LIMIT                          fc::minutes(60)
#define STEEMIT_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM 1

#endif

#define STEEMIT_BLOCK_INTERVAL                  3
#define STEEMIT_BLOCKS_PER_YEAR                 (365*24*60*60/STEEMIT_BLOCK_INTERVAL)
#define STEEMIT_BLOCKS_PER_DAY                  (24*60*60/STEEMIT_BLOCK_INTERVAL)
#define STEEMIT_START_VESTING_BLOCK             (STEEMIT_BLOCKS_PER_DAY * 47)
#define STEEMIT_START_MINER_VOTING_BLOCK        (60*10/STEEMIT_BLOCK_INTERVAL)
#define STEEMIT_FIRST_CASHOUT_TIME              (fc::time_point_sec(1484478000))

#define STEEMIT_INIT_MINER_NAME                 "cyberfounder"
#define STEEMIT_NUM_INIT_MINERS                 1
#define STEEMIT_MAX_VOTED_WITNESSES             1
#define STEEMIT_MAX_MINER_WITNESSES             1
#define STEEMIT_MAX_RUNNER_WITNESSES            1
#define STEEMIT_MAX_MINERS                      (STEEMIT_MAX_VOTED_WITNESSES+STEEMIT_MAX_MINER_WITNESSES+STEEMIT_MAX_RUNNER_WITNESSES)
#define STEEMIT_HARDFORK_REQUIRED_WITNESSES     1
#define STEEMIT_MAX_TIME_UNTIL_EXPIRATION       (60*60)
#define STEEMIT_MAX_MEMO_SIZE                   2048
#define STEEMIT_MAX_PROXY_RECURSION_DEPTH       4
#define STEEMIT_VESTING_WITHDRAW_INTERVALS      104
#define STEEMIT_VESTING_WITHDRAW_INTERVAL_SECONDS (60*60*24*7) // 1 week per interval
#define STEEMIT_MAX_WITHDRAW_ROUTES             10
#define STEEMIT_SAVINGS_WITHDRAW_TIME        	(fc::days(3))
#define STEEMIT_SAVINGS_WITHDRAW_REQUEST_LIMIT  100
#define STEEMIT_VOTE_REGENERATION_SECONDS       (5*60*60*24) // 5 day
#define STEEMIT_MAX_VOTE_CHANGES                5
#define STEEMIT_UPVOTE_LOCKOUT                  (fc::minutes(1))
#define STEEMIT_REVERSE_AUCTION_WINDOW_SECONDS  (60*30) /// 30 minutes
#define STEEMIT_MIN_VOTE_INTERVAL_SEC           3

#define STEEMIT_MIN_ROOT_COMMENT_INTERVAL       (fc::seconds(60*5)) // 5 minutes
#define STEEMIT_MIN_REPLY_INTERVAL              (fc::seconds(20)) // 20 seconds
#define STEEMIT_POST_AVERAGE_WINDOW             (60*60*24u) // 1 day
#define STEEMIT_POST_MAX_BANDWIDTH              (4*STEEMIT_100_PERCENT) // 2 posts per 1 days, average 1 every 12 hours
#define STEEMIT_POST_WEIGHT_CONSTANT            (uint64_t(STEEMIT_POST_MAX_BANDWIDTH) * STEEMIT_POST_MAX_BANDWIDTH)

#define STEEMIT_MAX_ACCOUNT_WITNESS_VOTES       30

#define STEEMIT_100_PERCENT                     10000
#define STEEMIT_1_PERCENT                       (STEEMIT_100_PERCENT/100)
#define STEEMIT_DEFAULT_SBD_INTEREST_RATE       (10*STEEMIT_1_PERCENT) ///< 10% APR

#define STEEMIT_MINER_PAY_PERCENT               (STEEMIT_1_PERCENT) // 1%
#define STEEMIT_MIN_RATION                      100000
#define STEEMIT_MAX_RATION_DECAY_RATE           (1000000)
#define STEEMIT_FREE_TRANSACTIONS_WITH_NEW_ACCOUNT 100

#define STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS (60*60*24*7) ///< 1 week
#define STEEMIT_BANDWIDTH_PRECISION             1000000ll ///< 1 million
#define STEEMIT_MAX_COMMENT_DEPTH               5

#define STEEMIT_MAX_RESERVE_RATIO               (20000)

#define STEEMIT_MINING_REWARD                   asset( 1000, STEEM_SYMBOL )

#define STEEMIT_LIQUIDITY_TIMEOUT_SEC           (fc::seconds(60*60*24*7)) // After one week volume is set to 0
#define STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC (fc::seconds(60)) // 1 minute required on books to receive volume
#define STEEMIT_LIQUIDITY_REWARD_PERIOD_SEC     (60*60)
#define STEEMIT_LIQUIDITY_REWARD_BLOCKS         (STEEMIT_LIQUIDITY_REWARD_PERIOD_SEC/STEEMIT_BLOCK_INTERVAL)
#define STEEMIT_MIN_LIQUIDITY_REWARD            (asset( 1000*STEEMIT_LIQUIDITY_REWARD_BLOCKS, STEEM_SYMBOL )) // Minumum reward to be paid out to liquidity providers
#define STEEMIT_MIN_CONTENT_REWARD              asset( 1500, STEEM_SYMBOL )
#define STEEMIT_MIN_CURATE_REWARD               asset( 500, STEEM_SYMBOL )
#define STEEMIT_MIN_PRODUCER_REWARD             STEEMIT_MINING_REWARD
#define STEEMIT_MIN_POW_REWARD                  STEEMIT_MINING_REWARD

#define STEEMIT_ACTIVE_CHALLENGE_FEE            asset( 2000, STEEM_SYMBOL )
#define STEEMIT_OWNER_CHALLENGE_FEE             asset( 30000, STEEM_SYMBOL )
#define STEEMIT_ACTIVE_CHALLENGE_COOLDOWN       fc::hours(1)
#define STEEMIT_OWNER_CHALLENGE_COOLDOWN        fc::hours(1)



// 5ccc e802 de5f
// int(expm1( log1p( 1 ) / BLOCKS_PER_YEAR ) * 2**STEEMIT_APR_PERCENT_SHIFT_PER_BLOCK / 100000 + 0.5)
// we use 100000 here instead of 10000 because we end up creating an additional 9x for vesting
#define STEEMIT_APR_PERCENT_MULTIPLY_PER_BLOCK          ( (uint64_t( 0x5ccc ) << 0x20) \
                                                        | (uint64_t( 0xe802 ) << 0x10) \
                                                        | (uint64_t( 0xde5f )        ) \
                                                        )
// chosen to be the maximal value such that STEEMIT_APR_PERCENT_MULTIPLY_PER_BLOCK * 2**64 * 100000 < 2**128
#define STEEMIT_APR_PERCENT_SHIFT_PER_BLOCK             87

#define STEEMIT_APR_PERCENT_MULTIPLY_PER_ROUND          ( (uint64_t( 0x79cc ) << 0x20 ) \
                                                        | (uint64_t( 0xf5c7 ) << 0x10 ) \
                                                        | (uint64_t( 0x3480 )         ) \
                                                        )

#define STEEMIT_APR_PERCENT_SHIFT_PER_ROUND             83

// We have different constants for hourly rewards
// i.e. hex(int(math.expm1( math.log1p( 1 ) / HOURS_PER_YEAR ) * 2**STEEMIT_APR_PERCENT_SHIFT_PER_HOUR / 100000 + 0.5))
#define STEEMIT_APR_PERCENT_MULTIPLY_PER_HOUR           ( (uint64_t( 0x6cc1 ) << 0x20) \
                                                        | (uint64_t( 0x39a1 ) << 0x10) \
                                                        | (uint64_t( 0x5cbd )        ) \
                                                        )

// chosen to be the maximal value such that STEEMIT_APR_PERCENT_MULTIPLY_PER_HOUR * 2**64 * 100000 < 2**128
#define STEEMIT_APR_PERCENT_SHIFT_PER_HOUR              77

// These constants add up to GRAPHENE_100_PERCENT.  Each GRAPHENE_1_PERCENT is equivalent to 1% per year APY
// *including the corresponding 9x vesting rewards*
#define STEEMIT_CONTENT_APR_PERCENT             5813
#define STEEMIT_CURATE_APR_PERCENT              1937
#define STEEMIT_LIQUIDITY_APR_PERCENT            750
#define STEEMIT_PRODUCER_APR_PERCENT             750
#define STEEMIT_POW_APR_PERCENT                  750

#define STEEMIT_MIN_PAYOUT_SBD                  (asset(20,SBD_SYMBOL))

#define STEEMIT_SBD_STOP_PERCENT                (5*STEEMIT_1_PERCENT ) // Stop printing SBD at 5% Market Cap
#define STEEMIT_SBD_START_PERCENT               (2*STEEMIT_1_PERCENT) // Start reducing printing of SBD at 2% Market Cap

#define STEEMIT_MIN_ACCOUNT_NAME_LENGTH         3
#define STEEMIT_MAX_ACCOUNT_NAME_LENGTH         16

#define STEEMIT_MIN_PERMLINK_LENGTH             0
#define STEEMIT_MAX_PERMLINK_LENGTH             256
#define STEEMIT_MAX_WITNESS_URL_LENGTH          2048

#define STEEMIT_INIT_SUPPLY                     int64_t(43306176000) // 43.306.176 GOLOS
#define STEEMIT_MAX_SHARE_SUPPLY                int64_t(1000000000000000ll)
#define STEEMIT_MAX_SIG_CHECK_DEPTH             2

#define STEEMIT_MIN_TRANSACTION_SIZE_LIMIT      1024
#define STEEMIT_SECONDS_PER_YEAR                (uint64_t(60*60*24*365ll))

#define STEEMIT_SBD_INTEREST_COMPOUND_INTERVAL_SEC  (60*60*24*30)
#define STEEMIT_MAX_TRANSACTION_SIZE            (1024*64)
#define STEEMIT_MIN_BLOCK_SIZE_LIMIT            (STEEMIT_MAX_TRANSACTION_SIZE)
#define STEEMIT_MAX_BLOCK_SIZE                  (STEEMIT_MAX_TRANSACTION_SIZE*STEEMIT_BLOCK_INTERVAL*2000)
#define STEEMIT_BLOCKS_PER_HOUR                 (60*60/STEEMIT_BLOCK_INTERVAL)
#define STEEMIT_FEED_INTERVAL_BLOCKS            (STEEMIT_BLOCKS_PER_HOUR)
#define STEEMIT_FEED_HISTORY_WINDOW             (24*7) /// 7 days * 24 hours per day
#define STEEMIT_MAX_FEED_AGE                    (fc::days(7))
#define STEEMIT_MIN_FEEDS                       (STEEMIT_MAX_MINERS/3) /// protects the network from conversions before price has been established
#define STEEMIT_CONVERSION_DELAY                (fc::days(7))

#define STEEMIT_MIN_UNDO_HISTORY                10
#define STEEMIT_MAX_UNDO_HISTORY                10000

#define STEEMIT_MAX_URL_LENGTH                  127

#define GRAPHENE_CURRENT_DB_VERSION             "GPH2.4"

#define STEEMIT_IRREVERSIBLE_THRESHOLD          (75 * STEEMIT_1_PERCENT)

/**
 *  Reserved Account IDs with special meaning
 */
///@{
/// Represents the current witnesses
#define STEEMIT_MINER_ACCOUNT                   "miners"
/// Represents the canonical account with NO authority (nobody can access funds in null account)
#define STEEMIT_NULL_ACCOUNT                    "null"
/// Represents the canonical account with WILDCARD authority (anybody can access funds in temp account)
#define STEEMIT_TEMP_ACCOUNT                    "temp"
/// Represents the canonical account for specifying you will vote for directly (as opposed to a proxy)
#define STEEMIT_PROXY_TO_SELF_ACCOUNT           ""
///@}
