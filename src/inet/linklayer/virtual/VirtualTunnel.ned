//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.virtual;

import inet.common.SimpleModule;

//
// Creates a virtual network interface that tunnels traffic through an existing
// real network interface. This is the main component of ~VirtualInterface.
// Supports tunneling via Ethernet or IEEE 802.1Q protocols, with optional
// VLAN tagging for traffic separation.
//
// It can copy properties from the real interface or generate new ones (e.g., MAC address).
// Handles packet forwarding between the virtual interface and the underlying
// real interface, adding appropriate tags for routing and identification.
//
simple VirtualTunnel extends SimpleModule
{
    parameters:
        @class(VirtualTunnel);
        string interfaceTableModule;
        string realInterface;
        string protocol = default(vlanId != -1 ? "ieee8021qctag" : "");
        string address @enum("auto","copy") = default("copy");
        int vlanId = default(-1);
        bool steal = default(vlanId != -1);
        @display("i=block/cogwheel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
}
