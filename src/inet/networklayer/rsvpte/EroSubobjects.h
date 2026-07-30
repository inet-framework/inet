//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_EROSUBOBJECTS_H
#define __INET_EROSUBOBJECTS_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"

namespace inet {

// RFC 3209 Section 4.3.3.1 IPv4-prefix ERO subobject: L bit + Type (7 bits, =1 for an
// IPv4 prefix), Length (1, =8), Address (4), Prefix Length (1, fixed at 32 -- EroObj
// carries single router addresses, not prefixes), Reserved (1) -- 8 bytes per hop.
//
// RFC 5440 Section 7.9 reuses this encoding verbatim for PCEP's own ERO object, so the
// RSVP-TE and PCEP serializers share the subobject body here. Only the enclosing object
// HEADER differs between the two protocols (RSVP's Length/Class-Num/C-Type versus PCEP's
// Object-Class/Object-Type/Object-Length), and each serializer still writes its own.

inline void serializeEroSubobjects(MemoryOutputStream& stream, const EroVector& ero)
{
    for (const auto& hop : ero) {
        stream.writeByte((hop.L ? 0x80 : 0x00) | 0x01); // L bit + Type=1 (IPv4 prefix)
        stream.writeByte(8); // subobject length
        stream.writeIpv4Address(hop.node);
        stream.writeByte(32); // prefix length
        stream.writeByte(0); // reserved
    }
}

// `hopCount` comes from the enclosing object's length, which only the caller can read.
inline EroVector deserializeEroSubobjects(MemoryInputStream& stream, int hopCount)
{
    EroVector ero;
    for (int i = 0; i < hopCount; i++) {
        uint8_t typeByte = stream.readByte();
        stream.readByte(); // subobject length, assumed 8
        EroObj hop;
        hop.L = (typeByte & 0x80) != 0;
        hop.node = stream.readIpv4Address();
        stream.readByte(); // prefix length, assumed 32
        stream.readByte(); // reserved
        ero.push_back(hop);
    }
    return ero;
}

} // namespace inet

#endif
