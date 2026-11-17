// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::dtls;

// RFC 9146: the receiver knows the CID length; no length octet is sent here.
class Dtls12CidHeader extends FieldsChunk
{
    uint8_t contentType = 25;
    uint16_t legacyVersion = 0xfefd;
    uint16_t epoch = 1;
    uint64_t sequenceNumber = 0;
    uint8_t connectionId[];
    uint16_t length = 0;
}
