//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

//
// Represents a BPDU (Bridge PDU) used by the STP and RSTP protocols,
// as defined in the 802.1D-1998 specification.
//
class Bpdu extends FieldsChunk
{
    chunkLength = B(35);
    uint16_t protocolIdentifier;              // 2 bytes, 0 for STP, 1 for RSTP (TODO use them)
    unsigned int protocolVersionIdentifier;   // 1 byte, version ID, currently 0

    unsigned int bpduType;                    // 1 byte, 0 for Configuration BPDU, 1 for Topology Change Notification BPDU

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

