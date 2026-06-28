//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This implementation is based on the IS-IS model of the ANSA project
// (https://ansa.omnetpp.org), Brno University of Technology, ported to the
// modern INET packet (Chunk) API.
//

package inet.routing.isis;

import inet.common.SimpleModule;
import inet.routing.contract.IIsis;

//
// Implements the IS-IS link-state interior gateway routing protocol
// (ISO/IEC 10589). IP reachability follows the Integrated IS-IS scheme
// (RFC 1195), installing routes into the regular IP routing table.
//
// IS-IS Protocol Data Units (IIH Hellos, Link-State PDUs, CSNPs and PSNPs) are
// not carried over IP; they ride directly on the link layer using IEEE 802.2
// LLC framing (SAP 0xFE) addressed to the AllL1ISs (01:80:C2:00:00:14) and
// AllL2ISs (01:80:C2:00:00:15) OSI multicast MAC addresses. The module
// therefore connects to the node's link-layer message dispatcher and drives an
// `Ieee8022LlcSocket` per participating interface.
//
simple Isis extends SimpleModule like IIsis
{
    parameters:
        @class(Isis);
        @display("i=block/network");

        string interfaceTableModule;             // path to the InterfaceTable module
        string routingTableModule = default(""); // path to the IRoutingTable to install routes into (Integrated IS-IS)

        xml configData = default(xml("<config/>")); // NET address, IS levels, and per-interface IS-IS settings

        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);
        double stopOperationTimeout @unit(s) = default(2s);
    gates:
        input socketIn;
        output socketOut;
}
