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

extern "C" {
#include "picotls.h"
#include "picotls/openssl_opp.h"
}

namespace inet {
namespace quic {

void InitialConnectionState::generateAndSetTempDstConnectionId()
{
    uint64_t tempCid1 = context->getModule()->intrand(UINT32_MAX);
    uint64_t tempCid2 = context->getModule()->intrand(UINT32_MAX);
    uint64_t tempCid = tempCid1 * tempCid2;
    context->addDstConnectionId(tempCid, 8);
    setConnectionKeysFromInitialRandom(tempCid);
}

void InitialConnectionState::setConnectionKeysFromInitialRandom(uint64_t dstConnId)
{
    ptls_iovec_t dcid_iovec = ptls_iovec_init(&dstConnId, 8);
    for (int i = 0; i < 4; ++i) {
        std::swap(dcid_iovec.base[i], dcid_iovec.base[7 - i]);
    }

    context->egressKeys[0] = EncryptionKey::newInitial(dcid_iovec, context->is_server ? "server in" : "client in");
    context->ingressKeys[0] = EncryptionKey::newInitial(dcid_iovec, context->is_server ? "client in" : "server in");

    std::cout << "initial egressKey of " << (context->is_server ? "server" : "client") << ": " << std::endl;
    context->egressKeys[0].dump();
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
{        Ptr<const Chunk> payload = pkt->popAtFront();

        if (auto tlsPayload = dynamicPtrCast<const BytesChunk>(payload)) {
            EV_DEBUG << "got transport parameter bytes" << endl;
            context->handleCryptoData(EncryptionLevel::Initial, tlsPayload->getBytes());
        }
    if (frameHeader->getContainsTransportParameters()) {
        if (auto transportParametersExt = dynamicPtrCast<const TransportParametersExtension>(payload)) {
            EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
            context->initializeRemoteTransportParameters(transportParametersExt);
        }
    }
}

ConnectionState *InitialConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processInitialPacket in " << name << endl;

    ackElicitingPacket = false;
    processFrames(pkt, PacketNumberSpace::Initial);

    context->addDstConnectionId(packetHeader->getSrcConnectionId(), packetHeader->getSrcConnectionIdLength());
    context->accountReceivedPacket(packetHeader->getPacketNumber(), ackElicitingPacket, PacketNumberSpace::Initial, false);
    setConnectionKeysFromInitialRandom(packetHeader->getDstConnectionId());

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
