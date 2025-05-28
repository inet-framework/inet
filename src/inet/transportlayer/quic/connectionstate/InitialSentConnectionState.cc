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
        Ptr<const Chunk> payload = pkt->popAtFront();
        if (auto transportParametersExt = dynamicPtrCast<const TransportParametersExtension>(payload)) {
            EV_DEBUG << "got transport parameters: " << transportParametersExt << endl;
            context->initializeRemoteTransportParameters(transportParametersExt);
        }
        if (auto tlsPayload = dynamicPtrCast<const BytesChunk>(payload)) {
            EV_DEBUG << "got transport parameter bytes" << endl;
            ptls_buffer_t buffer;
            ptls_buffer_init(&buffer, (void*)"", 0);
            size_t epoch_offsets[5] = {0};
            std::vector<uint8_t> tlsBytes = tlsPayload->getBytes();
            ptls_handle_message(context->tls, &buffer, epoch_offsets, 0,
                tlsBytes.data(), tlsBytes.size(), nullptr);
            for (int epoch = 0; epoch < 4; epoch++) {
                Ptr<BytesChunk> data = makeShared<BytesChunk>();
                size_t len = epoch_offsets[epoch + 1] - epoch_offsets[epoch];
                if (len == 0)
                    continue;
                std::vector<uint8_t> bytes(len);
                memcpy(bytes.data(), (uint8_t *)(buffer.base + epoch_offsets[epoch]), len);
                data->setBytes(bytes);
                context->newCryptoData((EncryptionLevel)epoch, data);
            }
        }
    }
}

void InitialSentConnectionState::processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace)
{
    context->getReliabilityManager()->onAckReceived(frameHeader, PacketNumberSpace::Initial);
}

} /* namespace quic */
} /* namespace inet */
