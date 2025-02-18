/*
 * InitialSentConnectionState.cc
 *
 *  Created on: 18 Feb 2025
 *      Author: msvoelker
 */


#include "HandshakeConnectionState.h"
#include "EstablishedConnectionState.h"

namespace inet {
namespace quic {


ConnectionState *HandshakeConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processHandshakePacket in " << name << endl;
    processFrames(pkt);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Handshake, false);

    return new EstablishedConnectionState(context);
}

} /* namespace quic */
} /* namespace inet */
