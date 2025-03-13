//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.contract;

//
// Defines the interface for all wired network interfaces in the INET framework.
// This interface extends the base network interface with specific requirements
// for physical connections over wired media such as twisted pair, coaxial,
// or fiber optic cables.
//
// Wired interfaces are characterized by:
// - Bidirectional physical connection through the 'phys' gate
// - Typically higher reliability and bandwidth compared to wireless interfaces
// - Lower latency and less susceptibility to environmental interference
//
// Common implementations include Ethernet interfaces (10/100/1000 Mbps),
// PPP interfaces for point-to-point links, and specialized industrial
// network interfaces.
//
// @see ~INetworkInterface, ~IWirelessInterface, ~IEthernetInterface, ~IPppInterface
//
moduleinterface IWiredInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        inout phys;
}
