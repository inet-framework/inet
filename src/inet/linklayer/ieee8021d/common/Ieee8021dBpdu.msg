//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

enum BpduProtocolIdentifier {
    SPANNING_TREE_PROTOCOL = 0;
};

enum BpduProtocolVersionIdentifier {
    SPANNING_TREE = 0;
    RAPID_SPANNING_TREE = 2;
    MULTIPLE_SPANNING_TREE = 3;
};

enum BpduType {
    BPDU_CFG = 0;
    BPDU_TCN = 0x80;
    BPDU_RAPID_OR_MULTIPLE_SPANNING_TREE = 0x02;
};

//
// Represents a BPDU (Bridge PDU) used by the STP and RSTP protocols,
// as defined in the 802.1D-1998 specification.
//
class BpduBase extends FieldsChunk
{
    BpduProtocolIdentifier protocolIdentifier = static_cast<BpduProtocolIdentifier>(-1);    // 2 bytes, 0 for STP, 1 for RSTP (TODO use them)
    BpduProtocolVersionIdentifier protocolVersionIdentifier = static_cast<BpduProtocolVersionIdentifier>(-1);    // 1 byte, version ID, currently 0, // 3:MultipleSpanningTree
    BpduType bpduType = static_cast<BpduType>(-1);    // 1 byte, 0 for Configuration BPDU, 1 for Topology Change Notification BPDU
}

class BpduTcn extends BpduBase
{
    chunkLength = B(4);
    protocolIdentifier = SPANNING_TREE_PROTOCOL;
    protocolVersionIdentifier = SPANNING_TREE;
    bpduType = BPDU_TCN;                            // Topology Change Notification BPDU
}

class BpduCfg extends BpduBase
{
    chunkLength = B(35);
    protocolIdentifier = SPANNING_TREE_PROTOCOL;
    protocolVersionIdentifier = SPANNING_TREE;
    bpduType = BPDU_CFG;
                                              // flags: 1 byte
    bool tcaFlag;                             //   Topology Change Acknowledgment flag
    uint8_t reserved = 0;                     //   unused 6 bits of Flags
    bool tcFlag;                              //   Topology Change flag

    uint16_t rootPriority;                    // 2 bytes, priority of the tree root (part of Root Identifier)
    MacAddress rootAddress;                   // 6 bytes, address of the tree root (part of Root Identifier)
    uint32_t rootPathCost;                    // 4 bytes, cost to the root

    uint16_t bridgePriority;                  // 2 bytes, priority of sender bridge (part of Bridge Identifier)
    MacAddress bridgeAddress;                 // 6 bytes, address of sender bridge (part of Bridge Identifier)

    unsigned int portPriority;                // 1 byte, priority of sender port (part of Port Identifier)
    unsigned int portNum;                     // 1 byte, port number (ethg[] gate index) of sender port (part of Port Identifier)
                                              // (or 4 bit priority (shifted with 4 bit) and 12 bit portnum (in 802.1D-2004))

    simtime_t messageAge;                     // 2 bytes, Message Age (in 256ths of a second)
    simtime_t maxAge;                         // 2 bytes, maximum lifetime of the BPDU (in 256ths of a second)
    simtime_t helloTime;                      // 2 bytes, Hello Time of the sender bridge (in 256ths of a second)
    simtime_t forwardDelay;                   // 2 bytes, Forward Delay timer of the sender bridge (in 256ths of a second)
}
