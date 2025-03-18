/*
 * InitialConnectionState.cc
 *
 *  Created on: 7 Feb 2025
 *      Author: msvoelker
 */


#include "InitialConnectionState.h"
#include "InitialSentConnectionState.h"
#include "EstablishedConnectionState.h"
#include "../packet/ConnectionId.h"

namespace inet {
namespace quic {


ConnectionState *InitialConnectionState::processConnectAppCommand(cMessage *msg)
{
    // send client hello
    context->sendClientInitialPacket();

    return new InitialSentConnectionState(context);
}


ConnectionState *InitialConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processInitialPacket in " << name << endl;

    processFrames(pkt, PacketNumberSpace::Initial);

    context->addDstConnectionId(packetHeader->getDstConnectionId(), packetHeader->getDstConnectionIdLength());
    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);

    // send server hello
    context->sendServerInitialPacket();

    // send Encrypted Extensions, Certificate, Certificate Verify, and Finished
    context->sendHandshakePacket();

    return new EstablishedConnectionState(context);
}

} /* namespace quic */
} /* namespace inet */
