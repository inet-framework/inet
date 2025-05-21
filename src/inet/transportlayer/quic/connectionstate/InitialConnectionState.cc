//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "InitialConnectionState.h"
#include "InitialSentConnectionState.h"
#include "EstablishedConnectionState.h"
#include "../packet/ConnectionId.h"
#include "../exception/InvalidTokenException.h"

namespace inet {
namespace quic {

void InitialConnectionState::generateAndSetTempDstConnectionId()
{
    uint64_t tempCid1 = context->getModule()->intrand(UINT32_MAX);
    uint64_t tempCid2 = context->getModule()->intrand(UINT32_MAX);
    uint64_t tempCid = tempCid1 * tempCid2;
    context->addDstConnectionId(tempCid, 8);
}

ConnectionState *InitialConnectionState::processConnectAppCommand(cMessage *msg)
{
    generateAndSetTempDstConnectionId();

    // send client hello
    context->sendClientInitialPacket();

    return new InitialSentConnectionState(context);
}

ConnectionState *InitialConnectionState::processConnectAndSendAppCommand(cMessage *msg)
{
    Packet *pkt = check_and_cast<Packet *>(msg);
    const char *clientToken = pkt->getTag<QuicNewToken>()->getToken();

    uint32_t token = 0;
    try {
        // try to parse the given client token
        token = context->processClientTokenExtractToken(clientToken);
    } catch (InvalidTokenException& e) {
        // parsing of client token failed, token remains 0
        EV_WARN << e.what() << endl;
    }

    generateAndSetTempDstConnectionId();

    // send client Initial Packet with token if it is larger than 0
    context->sendClientInitialPacket(token);

    auto streamId = pkt->getTag<QuicStreamReq>()->getStreamID();
    context->newStreamData(streamId, pkt->peekData());

    return new InitialSentConnectionState(context);
}

void InitialConnectionState::processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt)
{
    if (frameHeader->getContainsTransportParameters()) {
        auto transportParametersExt = staticPtrCast<const TransportParametersExtension>(pkt->popAtFront());
        EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
        context->initializeRemoteTransportParameters(transportParametersExt);
    }
}

ConnectionState *InitialConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processInitialPacket in " << name << endl;

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::Initial);

    context->addDstConnectionId(packetHeader->getSrcConnectionId(), packetHeader->getSrcConnectionIdLength());
    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);

    if (packetHeader->getTokenLength() > 0) {
        uint32_t token = packetHeader->getToken();
        if (context->getUdpSocket()->doesTokenExist(token, context->getPath()->getRemoteAddr())) {
            EV_DEBUG << "InitialConnectionState::processInitialPacket: Found contained token, address validated" << endl;
            context->addConnectionForInitialConnectionId(packetHeader->getDstConnectionId());
        } else {
            EV_WARN << "InitialConnectionState::processInitialPacket: Initial packet contains an invalid token, ignore 0-RTT packets" << endl;
            // without adding the connection ID, the 0-RTT packets will not be assigned to this connection and will be discarded
        }
    }
    // send server hello
    context->sendServerInitialPacket();

    // send Encrypted Extensions, Certificate, Certificate Verify, and Finished
    context->sendHandshakePacket(true);

    return new EstablishedConnectionState(context);
}

} /* namespace quic */
} /* namespace inet */
