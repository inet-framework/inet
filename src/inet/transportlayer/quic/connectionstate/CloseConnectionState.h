//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_CLOSECONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_CLOSECONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class CloseConnectionState: public ConnectionState {
public:
    CloseConnectionState(Connection *context) : ConnectionState(context) {
        name = "Close";
    }

    virtual ConnectionState *processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt) override;
    virtual ConnectionState *processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) override;
    virtual ConnectionState *processConnectionCloseTimeout(cMessage *msg) override;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_CLOSECONNECTIONSTATE_H_ */
