//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.queueing.base.PacketFilterBase;

//
// Verifies and processes Ethernet MAC headers in incoming frames. Extracts
// source and destination MAC addresses, determines the encapsulated protocol
// based on the EtherType/Length field, and adds appropriate tags to the packet.
// In normal mode, filters out frames not addressed to the interface, while in
// promiscuous mode accepts all frames regardless of destination address.
//
simple EthernetMacHeaderChecker extends PacketFilterBase like IProtocolHeaderChecker
{
    parameters:
        string interfaceTableModule;
        bool promiscuous = default(false);
        @class(EthernetMacHeaderChecker);
        @display("i=block/checker");
}
