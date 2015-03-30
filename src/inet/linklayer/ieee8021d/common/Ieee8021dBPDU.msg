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

cplusplus {{
#include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

class noncobject MACAddress;


//
// Represents a BPDU (Bridge PDU) used by the STP and RSTP protocols,
// as defined in the 802.1D-1998 specification.
//
packet BPDU
{
    byteLength = 35;
    unsigned int protocolIdentifier;          // 0 for STP, 1 for RSTP (TODO use them)
    unsigned int protocolVersionIdentifier;   // currently 0

    unsigned int bpduType;                    // 0 for Configuration BPDU, 1 for Topology Change Notification BPDU
    bool tcaFlag;                             // Topology Change Acknowledgment flag
    bool tcFlag;                              // Topology Change flag

    MACAddress rootAddress;                   // address of the tree root (first part of Root Identifier)
    unsigned int rootPriority;                // priority of the tree root (second part of Root Identifier)
    unsigned int rootPathCost;                // cost to the root

    MACAddress bridgeAddress;                 // address of sender bridge (first part of Bridge Identifier)
    unsigned int bridgePriority;              // priority of sender bridge (second part of Bridge Identifier)

    unsigned int portNum;                     // port number (ethg[] gate index) of sender port (first part of Port Identifier)
    unsigned int portPriority;                // priority of sender port (second part of Port Identifier)

    simtime_t messageAge;                     // Message Age in seconds
    simtime_t maxAge;                         // maximum lifetime of the BPDU (seconds)
    simtime_t helloTime;                      // Hello Time of the sender bridge
    simtime_t forwardDelay;                   // Forward Delay timer of the sender bridge
}
