//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "InitialSentConnectionState.h"
#include "HandshakeConnectionState.h"

namespace inet {
namespace quic {

ConnectionState *InitialSentConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processInitialPacket in " << name << endl;

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::Initial);

    // delete temporary destination connection id
    context->clearDstConnectionIds();
    context->addDstConnectionId(packetHeader->getSrcConnectionId(), packetHeader->getSrcConnectionIdLength());
    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);
    context->sendAck(PacketNumberSpace::Initial);

    return new HandshakeConnectionState(context);
}

void InitialSentConnectionState::processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt)
{
    if (frameHeader->getContainsTransportParameters()) {
        auto transportParametersExt = staticPtrCast<const TransportParametersExtension>(pkt->popAtFront());
        EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
        context->initializeRemoteTransportParameters(transportParametersExt);
    }
}

void InitialSentConnectionState::processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace)
{
    context->getReliabilityManager()->onAckReceived(frameHeader, PacketNumberSpace::Initial);
}

} /* namespace quic */
} /* namespace inet */
