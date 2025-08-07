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
{        Ptr<const Chunk> payload = pkt->popAtFront();
    std::cout << "HandshakeConnectionState::processCryptoFrame at " << (context->is_server ? "server" : "client") << ", payload: " << payload << std::endl;
        if (auto tlsPayload = dynamicPtrCast<const BytesChunk>(payload)) {
            EV_DEBUG << "got transport parameter bytes" << endl;
            context->handleCryptoData(EncryptionLevel::Handshake, tlsPayload->getBytes());
        }
    if (frameHeader->getContainsTransportParameters()) {
        if (auto transportParametersExt = dynamicPtrCast<const TransportParametersExtension>(payload)) {
            EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
            context->initializeRemoteTransportParameters(transportParametersExt);
        }
    }
    else {
        EV_DEBUG << "HandshakeConnectionState::processCryptoFrame - no transport parameters" << endl;
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
