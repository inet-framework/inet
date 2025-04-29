/*
 * InitialSentConnectionState.cc
 *
 *  Created on: 18 Feb 2025
 *      Author: msvoelker
 */


#include "InitialSentConnectionState.h"
#include "HandshakeConnectionState.h"

namespace inet {
namespace quic {

ConnectionState *InitialSentConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processInitialPacket in " << name << endl;
    processFrames(pkt, PacketNumberSpace::Initial);

    context->addDstConnectionId(packetHeader->getDstConnectionId(), packetHeader->getDstConnectionIdLength());
    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);
    context->sendAck(PacketNumberSpace::Initial);

    return new HandshakeConnectionState(context);
}

void InitialSentConnectionState::processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt)
{
    if (frameHeader->getContainsTransportParameters()) {
        auto transportParametersExt = staticPtrCast<const TransportParametersExtension>(pkt->popAtFront());
        EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
        context->getRemoteTransportParameters()->readExtension(transportParametersExt);
    }
}

void InitialSentConnectionState::processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace)
{
    context->getReliabilityManager()->onAckReceived(frameHeader, PacketNumberSpace::Initial);
}

} /* namespace quic */
} /* namespace inet */
