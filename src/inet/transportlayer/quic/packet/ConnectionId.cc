//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ConnectionId.h"
#include <cmath>

namespace inet {
namespace quic {

ConnectionId::ConnectionId(uint64_t id, uint8_t length) {
    this->id = id;
    this->length = length;
}

ConnectionId::ConnectionId(uint64_t id) {
    this->id = id;
    length = 1;
    if (id > 0) {
        length += (uint8_t) (log2(id)/8);
    }
}

} /* namespace quic */
} /* namespace inet */

