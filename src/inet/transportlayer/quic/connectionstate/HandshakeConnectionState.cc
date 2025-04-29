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

void HandshakeConnectionState::processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt)
{
    if (frameHeader->getContainsTransportParameters()) {
        auto transportParametersExt = staticPtrCast<const TransportParametersExtension>(pkt->popAtFront());
        EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
        context->getRemoteTransportParameters()->readExtension(transportParametersExt);
    }
}

ConnectionState *HandshakeConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processHandshakePacket in " << name << endl;
    processFrames(pkt, PacketNumberSpace::Handshake);

    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Handshake, false);

    // send Finished
    context->sendHandshakePacket(false);

    return new EstablishedConnectionState(context);
}

} /* namespace quic */
} /* namespace inet */
