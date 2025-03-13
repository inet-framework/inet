//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.queueing.base.PacketFilterBase;

//
// Verifies and processes the MAC header of incoming Ethernet frames. This module
// is responsible for validating the header structure and filtering frames based
// on destination MAC address.
//
// The module performs several key functions in the Ethernet protocol stack:
// - Validates the structure and format of the Ethernet MAC header
// - Extracts source and destination MAC addresses from the header
// - Filters frames based on the destination address and interface configuration
// - Processes and removes the MAC header from valid frames
// - Updates MAC address learning tables with source address information
//
// In normal operation, the module accepts frames that:
// - Have a valid MAC header structure
// - Are addressed to the interface's MAC address
// - Are broadcast frames (destination FF:FF:FF:FF:FF:FF)
// - Are multicast frames that the interface is configured to receive
//
// When operating in promiscuous mode, the module accepts all frames regardless
// of destination address, which is useful for network monitoring and analysis.
//
// This module is typically used within modular Ethernet interfaces as part of the
// MAC layer processing pipeline, paired with the EthernetMacHeaderInserter module
// that adds MAC headers to outgoing frames.
//
// @see ~EthernetMacLayer, ~EthernetMacHeaderInserter, ~LayeredEthernetInterface
//
simple EthernetMacHeaderChecker extends PacketFilterBase like IProtocolHeaderChecker
{
    parameters:
        string interfaceTableModule;  // Path to the InterfaceTable module to obtain the interface's MAC address
        bool promiscuous = default(false);  // When true, accepts all frames regardless of destination address
        @class(EthernetMacHeaderChecker);
        @display("i=block/checker");
}
