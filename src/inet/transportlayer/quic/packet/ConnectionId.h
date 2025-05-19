//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_PACKET_CONNECTIONID_H_
#define INET_TRANSPORTLAYER_QUIC_PACKET_CONNECTIONID_H_

#include <stdint.h>

namespace inet {
namespace quic {

class ConnectionId {
public:
    ConnectionId(uint64_t id, uint8_t length);
    ConnectionId(uint64_t id);

    uint64_t getId() {
        return id;
    }
    uint8_t getLength() {
        return length;
    }

private:
    uint64_t id;
    uint8_t length;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PACKET_CONNECTIONID_H_ */
