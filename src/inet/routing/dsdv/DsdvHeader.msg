// SPDX-License-Identifier: LGPL-3.0-or-later
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet::dsdv;

struct DsdvEntry
{
    @packetData;
    Ipv4Address destination;
    uint64_t sequence = 0;
    uint32_t metric = 0;
    bool defaultCapability = false; // Version 2 flag bit 0, finite base-station routes only
}

// Model wire profile: 16B header, 16B v1 entry or 17B v2 entry, network order.
// Kind 1 = full dump fragment; kind 2 = independently processable incremental.
class DsdvHeader extends FieldsChunk
{
    uint8_t version = 1;
    uint8_t kind = 1;
    Ipv4Address sender;
    uint64_t selfSequence = 0;
    DsdvEntry entries[];
}
