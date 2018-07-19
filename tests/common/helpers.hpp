#pragma once

// Common code sequences for testing and boost container extensions


// Validate
//-------------------------------------------------------------

/// validate operation
#define CHECK_OP_VALID(OP) BOOST_CHECK_NO_THROW(OP.validate())

/// change operation parameter value and check it's still valid
#define CHECK_PARAM_VALID(OP, NAME, VAL) \
    CHECK_PARAM_VALID_I(OP, NAME, VAL, CHECK_OP_VALID(OP))

/// change operation parameter value and check it's invalid (+restore param)
#define CHECK_PARAM_INVALID(OP, N, V) \
    CHECK_PARAM_VALIDATION_FAIL(OP, N, V, invalid_parameter, #N)

/// change operation parameter value and check it fails with logic_exceprion (+restore param)
#define CHECK_PARAM_INVALID_LOGIC(OP, N, V, EX) \
    CHECK_PARAM_VALIDATION_FAIL(OP, N, V, logic_exception, logic_exception:: EX)

/// same as previous but with configurable CHECK_ERROR parameters
#define CHECK_PARAM_VALIDATION_FAIL(OP, N, V, EX, EV) \
    CHECK_PARAM_VALID_I(OP, N, V, \
        GOLOS_CHECK_ERROR_PROPS(OP.validate(), CHECK_ERROR(EX, EV)))


// Push tx
#define GOLOS_TEST_TX_THROW(E, KEY, SETUP) GOLOS_TEST_TX_I(SETUP, KEY, GOLOS_CHECK_PUSH_TX_THROW(E))
#define GOLOS_TEST_TX_NO_THROW(KEY, SETUP) GOLOS_TEST_TX_I(SETUP, KEY, GOLOS_CHECK_PUSH_TX_NO_THROW)
#define GOLOS_TEST_TX_I(SETUP, KEY, PUSH) \
    tx.clear(); \
    SETUP; \
    tx.sign(KEY, db->get_chain_id()); \
    PUSH;


// Check operation authorities
#define CHECK_OP_AUTHS(OP, OWNER, ACTIVE, POSTING) {\
    account_name_set auths;                         \
    OP.get_required_owner_authorities(auths);       \
    BOOST_CHECK_EQUAL(auths, OWNER);                \
    auths.clear();                                  \
    OP.get_required_active_authorities(auths);      \
    BOOST_CHECK_EQUAL(auths, ACTIVE);               \
    auths.clear();                                  \
    OP.get_required_posting_authorities(auths);     \
    BOOST_CHECK_EQUAL(auths, POSTING);              \
    auths.clear();                                  \
}


// internals
//-------------------------------------------------------------

// validate
#define CHECK_PARAM_VALID_I(OP, NAME, VAL, CHECK) { \
    auto t = OP.NAME; \
    OP.NAME = VAL; \
    CHECK; \
    OP.NAME = t; \
}


// push tx
#define GOLOS_CHECK_PUSH_TX_NO_THROW GOLOS_CHECK_PUSH_TX_NO_THROW_F(0)
#define GOLOS_CHECK_PUSH_TX_NO_THROW_F(F) BOOST_CHECK_NO_THROW(db->push_transaction(tx, F))
#define GOLOS_CHECK_PUSH_TX_THROW(E) GOLOS_CHECK_PUSH_TX_THROW_F(0, E)
#define GOLOS_CHECK_PUSH_TX_THROW_F(F, E) \
    GOLOS_CHECK_ERROR_PROPS(db->push_transaction(tx, F), CHECK_ERROR(E, 0))


// boost::container <<
//-------------------------------------------------------------
namespace boost { namespace container {

template<typename T>
std::ostream &operator<<(std::ostream &out, const flat_set<T> &t) {
    out << "(";
    if (!t.empty()) {
        std::for_each(t.begin(), t.end()-1, [&](const T& v) {out << v << ",";});
        out << *t.rbegin();
    }
    out << ")";
    return out;
}

} } // namespace boost::container
