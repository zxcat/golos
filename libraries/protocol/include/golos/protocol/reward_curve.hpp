#pragma once

#include <golos/protocol/base.hpp>

namespace golos { namespace protocol {

    enum class curation_curve: uint8_t {
        detect, ///< get from current settings
        reverse_auction,
        fractional,
        linear,
        square_root,
    }; // enum curation_curve

} } // namespace golos::protocol

FC_REFLECT_ENUM(golos::protocol::curation_curve, (detect)(reverse_auction)(fractional)(linear)(square_root))
