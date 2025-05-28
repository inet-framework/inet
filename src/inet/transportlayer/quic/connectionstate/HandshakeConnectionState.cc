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
            ptls_handle_message(context->tls, &buffer, epoch_offsets, 2,
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
