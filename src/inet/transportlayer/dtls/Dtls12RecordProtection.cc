// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls12RecordProtection.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/transportlayer/dtls/Dtls12HelloVerifyRequest_m.h"

namespace inet::dtls {

uint64_t Dtls12RecordProtection::explicitNonce(uint16_t epoch, uint64_t sequence)
{
    if (epoch == 0 || sequence >= RECORD_LIMIT)
        throw cRuntimeError("Invalid DTLS 1.2 protected epoch or sequence");
    return (uint64_t(epoch) << 48) | sequence;
}

void Dtls12RecordProtection::protect(Packet *packet, uint16_t epoch, uint64_t sequence)
{
    auto nonceValue = explicitNonce(epoch, sequence);
    auto length = packet->getDataLength();
    if (length.get<b>() % 8 != 0 || length > B(MAX_CONTENT_BYTES))
        throw cRuntimeError("Invalid DTLS 1.2 plaintext length");
    auto tags = packet->getRegionTags();
    auto offset = packet->getFrontOffset();
    if (length > b(0)) {
        auto content = packet->removeData();
        content->markImmutable();
        packet->insertData(makeShared<EncryptedChunk>(content, length));
    }
    auto nonce = makeShared<Dtls12ExplicitNonce>();
    nonce->setValue(nonceValue);
    packet->insertAtFront(nonce);
    packet->insertAtBack(makeShared<ByteCountChunk>(B(TAG_BYTES)));
    if (length > b(0))
        packet->getRegionTags().copyTags(tags, offset, packet->getFrontOffset() + B(NONCE_BYTES), length);
}

bool Dtls12RecordProtection::unprotect(Packet *packet, uint16_t epoch, uint64_t sequence)
{
    if (epoch == 0 || sequence >= RECORD_LIMIT || packet->getDataLength().get<b>() % 8 != 0 ||
        packet->getDataLength() < B(NONCE_BYTES + TAG_BYTES) || packet->getDataLength() > B(MAX_CONTENT_BYTES + NONCE_BYTES + TAG_BYTES))
        return false;
    try {
        Packet retained(*packet);
        auto nonce = retained.popAtFront<Dtls12ExplicitNonce>(B(NONCE_BYTES));
        if (nonce->getValue() != explicitNonce(epoch, sequence))
            return false;
        auto length = retained.getDataLength() - B(TAG_BYTES);
        Ptr<const Chunk> content;
        if (length > b(0)) {
            auto encrypted = retained.peekAtFront<EncryptedChunk>(length);
            if (!encrypted->getChunk() || encrypted->getChunkLength() != length || encrypted->getChunk()->getChunkLength() != length)
                return false;
            content = encrypted->getChunk();
        }
        auto tags = packet->getRegionTags();
        auto offset = packet->getFrontOffset() + B(NONCE_BYTES);
        packet->removeData();
        if (content) {
            packet->insertData(content);
            packet->getRegionTags().copyTags(tags, offset, packet->getFrontOffset(), length);
        }
        return true;
    }
    catch (const cRuntimeError&) {
        return false;
    }
}

} // namespace inet::dtls
