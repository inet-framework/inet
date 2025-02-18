/*
 * ConnectionId.cpp
 *
 *  Created on: 18 Feb 2025
 *      Author: msvoelker
 */

#include "ConnectionId.h"

namespace inet {
namespace quic {

ConnectionId::ConnectionId(uint64_t id, uint8_t length) {
    this->id = id;
    this->length = length;
}

} /* namespace quic */
} /* namespace inet */

