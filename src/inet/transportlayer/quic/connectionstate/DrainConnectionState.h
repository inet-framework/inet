//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_DRAINCONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_DRAINCONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class DrainConnectionState: public ConnectionState {
public:
    DrainConnectionState(Connection *context) : ConnectionState(context) {
        name = "Drain";
    }

    virtual ConnectionState *processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt) override;
    virtual ConnectionState *processConnectionCloseTimeout(cMessage *msg) override;

    EncryptionLevel getEncryptionLevel() override { return EncryptionLevel::OneRtt; }
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_DRAINCONNECTIONSTATE_H_ */
