#pragma once

#include <steemit/chain/evaluator.hpp>

namespace steemit {
    namespace chain {

        struct transfer_to_blind_operation;
        struct transfer_from_blind_operation;
        struct blind_transfer_operation;

        class transfer_to_blind_evaluator : public evaluator<transfer_to_blind_evaluator> {
        public:
            typedef transfer_to_blind_operation operation_type;

            transfer_to_blind_evaluator(database &db) : evaluator<transfer_to_blind_evaluator>(db) {
            }

            void do_apply(const transfer_to_blind_operation &o);
        };

        class transfer_from_blind_evaluator : public evaluator<transfer_from_blind_evaluator> {
        public:
            typedef transfer_from_blind_operation operation_type;

            transfer_from_blind_evaluator(database &db) : evaluator<transfer_from_blind_evaluator>(db) {
            }

            void do_apply(const transfer_from_blind_operation &o);
        };

        class blind_transfer_evaluator : public evaluator<blind_transfer_evaluator> {
        public:
            typedef blind_transfer_operation operation_type;

            blind_transfer_evaluator(database &db) : evaluator<blind_transfer_evaluator>(db) {
            }

            void do_apply(const blind_transfer_operation &o);
        };
    }
} // namespace steemit::chain
