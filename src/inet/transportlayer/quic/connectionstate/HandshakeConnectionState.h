//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_HANDSHAKECONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_HANDSHAKECONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class HandshakeConnectionState: public ConnectionState {
public:
    HandshakeConnectionState(Connection *context) : ConnectionState(context) {
        name = "Handshake";
    }

    virtual ConnectionState *processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) override;
    virtual void processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt) override;

    EncryptionLevel getEncryptionLevel() override { return EncryptionLevel::Handshake; }
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_HANDSHAKECONNECTIONSTATE_H_ */
