//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "HandshakeConnectionState.h"
#include "EstablishedConnectionState.h"

namespace inet {
namespace quic {

void HandshakeConnectionState::processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt)
{
    if (frameHeader->getContainsTransportParameters()) {
        auto transportParametersExt = staticPtrCast<const TransportParametersExtension>(pkt->popAtFront());
        EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
        context->initializeRemoteTransportParameters(transportParametersExt);
    }
}

ConnectionState *HandshakeConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processHandshakePacket in " << name << endl;

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::Handshake);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Handshake, false);

    // send Finished
    context->sendHandshakePacket(false);

    return new EstablishedConnectionState(context);
}

} /* namespace quic */
} /* namespace inet */
