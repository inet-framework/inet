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
class CfmMessage extends FieldsChunk {
    uint8_t mdLevel = 0;
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
    chunkLength = B(75);
    opCode = 1;  // CCM
    uint32_t sequenceNumber = 0;
    uint16_t endpointIdentifier = 1; // MEP ID
    string messageName; // max 45 Bytes
}
