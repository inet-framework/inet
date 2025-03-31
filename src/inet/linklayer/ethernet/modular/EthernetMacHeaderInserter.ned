//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.base.PacketFlowBase;

//
// Creates and inserts Ethernet MAC headers into outgoing packets. Sets source
// and destination MAC addresses from packet tags, using the interface's MAC
// address if the source is unspecified. Determines the Type/Length field based
// on the encapsulated protocol, using either the protocol's EtherType value or
// the packet length for IEEE 802.3 frames. Updates packet tags to reflect the
// added header.
//
simple EthernetMacHeaderInserter extends PacketFlowBase like IProtocolHeaderInserter
{
    parameters:
        string interfaceTableModule;
        @class(EthernetMacHeaderInserter);
        @display("i=block/inserter");
}
