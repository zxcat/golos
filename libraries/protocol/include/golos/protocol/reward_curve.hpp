#pragma once

#include <golos/protocol/base.hpp>

namespace golos { namespace protocol {

    enum class curation_curve: uint8_t {
        bounded,
        linear,
        square_root,
        _size,
        detect = 100, ///< get from current settings
    }; // enum curation_curve

} } // namespace golos::protocol

FC_REFLECT_ENUM(golos::protocol::curation_curve, (detect)(bounded)(linear)(square_root)(_size))
