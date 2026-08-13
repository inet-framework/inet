//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// Base header for the Connectivity Fault Management (CFM) protocol suite for
// managing connectivity at the Ethernet service level. CFM is defined within
// the IEEE Std 802.1Q-2018 standard, later IEEE 802.1ag and ITU Y.1731.
//
// A TLV of a CFM PDU. Every one of them starts with a type octet; the one that ends the
// list is that octet alone, while the others -- Sender ID, Port Status, Interface Status
// and the organization-specific ones -- carry a two-octet length and a value after it.
// Deserializing this base yields the TLV the type names.
class CfmTlvBase extends FieldsChunk {
    chunkLength = B(1);
    uint8_t type;
}

// The TLV that ends the list (802.1ag 21.5.1): a type octet of zero, with neither a
// length nor a value after it.
class CfmEndTlv extends CfmTlvBase {
    type = 0;
}

// Any other TLV: the type, the length it declares and the octets that follow. What is
// inside them is not modelled, but with this every TLV of a CFM PDU reads back and
// writes out again as it was.
class CfmTlvRaw extends CfmTlvBase {
    uint16_t length;
    uint8_t bytes[];
}

class CfmMessage extends FieldsChunk {
    // the first octet carries both: the level in the high three bits, the version in the
    // low five
    uint8_t mdLevel @bit(3) = 0;
    uint8_t version @bit(5) = 0;
    uint8_t opCode;
    uint8_t flags = 0x00000001; // RDI 1 bit, Traffic 1 bit, Reserved 3 bit, CCM interval 3 bit
}

//
// Protocol header for the Continuity Check protocol (CC), which is a fundamental
// component of CFM.
//
// NOTE: Fields not used by the simulation are omitted.
//
class CfmContinuityCheckMessage extends CfmMessage {
    // Common header, sequence number and endpoint identifier, then the 48-octet MAID and
    // the 16 octets the ITU defines. The TLVs that follow -- and the End TLV that closes
    // them -- are not part of this, they are chunks of their own.
    chunkLength = B(74);
    opCode = 1;  // CCM
    uint32_t sequenceNumber = 0;
    uint16_t endpointIdentifier = 1; // MEP ID
    // The MAID names the maintenance domain and the maintenance association inside it.
    // Each name is preceded by the format it is written in, and a format of 1 says the
    // name it belongs to is absent; what is left of the 48 octets is zero.
    uint8_t mdNameFormat = 1;   // 1: no maintenance domain name
    string mdName;
    uint8_t maNameFormat = 2;   // 2: character string
    string maName;
}
