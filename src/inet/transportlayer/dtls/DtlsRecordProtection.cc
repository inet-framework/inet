// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/EncryptedChunk.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

void DtlsRecordProtection::protect(Packet *packet, uint8_t innerContentType)
{
    // RFC 9846 Section 5.2: model the inner type and tag with retained plaintext, not cryptography.
    auto payloadOffset = packet->getFrontOffset();
    auto payloadLength = packet->getDataLength();
    auto payloadTags = packet->getRegionTags();
    auto trailer = makeShared<DtlsInnerContentType>();
    trailer->setContentType(innerContentType);
    packet->insertAtBack(trailer);
    auto content = packet->removeData();
    content->markImmutable();
    packet->insertData(makeShared<EncryptedChunk>(content, content->getChunkLength()));
    packet->insertAtBack(makeShared<ByteCountChunk>(B(TAG_BYTES)));
    if (payloadLength > b(0))
        packet->getRegionTags().copyTags(payloadTags, payloadOffset, packet->getFrontOffset(), payloadLength);
}

bool DtlsRecordProtection::unprotect(Packet *packet, uint8_t& innerContentType)
{
    // RFC 9846 Sections 5.2 and 5.4: discard trailing zero padding before extracting the inner type.
    if (packet->getDataLength() < B(TAG_BYTES + 1))
        return false;
    try {
        auto encrypted = packet->peekAtFront<EncryptedChunk>(packet->getDataLength() - B(TAG_BYTES));
        if (!encrypted->getChunk() || encrypted->getChunkLength() != encrypted->getChunk()->getChunkLength())
            return false;
        Packet retained("DTLS retained plaintext", encrypted->getChunk());
        auto trailer = retained.popAtBack<DtlsInnerContentType>(B(1));
        auto content = retained.removeData(Chunk::PF_ALLOW_EMPTY);
        auto type = trailer->getContentType();
        auto payloadTags = packet->getRegionTags();
        auto payloadOffset = packet->getFrontOffset();
        packet->removeData();
        if (content->getChunkLength() > b(0)) {
            packet->insertData(content);
            packet->getRegionTags().copyTags(payloadTags, payloadOffset, packet->getFrontOffset(), content->getChunkLength());
        }
        innerContentType = type;
        return true;
    }
    catch (const cRuntimeError&) {
        return false;
    }
}

} // namespace inet::dtls
