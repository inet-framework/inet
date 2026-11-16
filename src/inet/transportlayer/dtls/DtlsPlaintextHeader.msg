//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.transportlayer.tls.TlsClientHello;

namespace inet::dtls;

// RFC 9147 section 4: epoch-zero record header.
class DtlsPlaintextHeader extends FieldsChunk
{
    chunkLength = B(13);
    uint8_t contentType = 22;
    uint16_t legacyVersion = 0xfefd;
    uint16_t epoch = 0;
    uint64_t sequenceNumber = 0;
    uint16_t length = 0;
}

// The unmasked fields carried by a unified header; record-number encryption is
// deliberately not modeled. sequenceNumber holds only the transmitted low bits.
class DtlsCiphertextHeader extends FieldsChunk
{
    chunkLength = B(5);
    uint8_t epochBits = 0;
    uint16_t sequenceNumber = 0;
    uint8_t sequenceNumberLength = 2;
    bool lengthPresent = true;
    uint16_t length = 0;
    uint8_t connectionId[];
}

// RFC 9147 section 5.2: the message sequence is distinct from record sequence.
class DtlsHandshakeHeader extends FieldsChunk
{
    chunkLength = B(12);
    uint8_t messageType = 0;
    uint32_t messageLength = 0;
    uint16_t messageSequence = 0;
    uint32_t fragmentOffset = 0;
    uint32_t fragmentLength = 0;
}

class DtlsClientHello extends inet::tls::TlsClientHello
{
    legacyVersion = 0xfefd;
    uint8_t legacyCookie[];
}

// The single inner-content-type octet is inside the modeled protected chunk.
class DtlsInnerContentType extends FieldsChunk
{
    chunkLength = B(1);
    uint8_t contentType = 0;
}

class DtlsAck extends FieldsChunk
{
    uint64_t epochs[];
    uint64_t sequenceNumbers[];
}

class DtlsAlert extends FieldsChunk
{
    chunkLength = B(2);
    uint8_t level = 2;
    uint8_t description = 0;
}
