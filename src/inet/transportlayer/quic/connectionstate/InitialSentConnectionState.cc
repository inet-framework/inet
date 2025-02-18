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
    processFrames(pkt);

    context->addDstConnectionId(packetHeader->getDstConnectionId(), packetHeader->getDstConnectionIdLength());
    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);

    context->sendHandshakePacket();

    return new HandshakeConnectionState(context);
}


void InitialSentConnectionState::processAckFrame(const Ptr<const AckFrameHeader>& frameHeader)
{
    context->getReliabilityManager()->onAckReceived(frameHeader, PacketNumberSpace::Initial);
}

} /* namespace quic */
} /* namespace inet */
