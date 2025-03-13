//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.virtual;

//
// Implements a virtual tunnel that connects to a real network interface and
// forwards packets between the virtual interface and the real interface.
// This module enables the creation of virtual interfaces that share the
// physical connectivity of a real interface, similar to virtual interfaces
// in operating systems.
//
// The tunnel can operate with different protocols (Ethernet, IEEE 802.1Q)
// and can optionally add VLAN tags to packets. It can either generate its
// own MAC address or copy the address from the real interface.
//
// Key parameters:
// - realInterface: Name of the real interface to connect to
// - protocol: Protocol to use (default is IEEE 802.1Q if VLAN is enabled)
// - address: MAC address mode ("auto" for generated, "copy" to use real interface's address)
// - vlanId: VLAN ID to use (-1 disables VLAN tagging)
// - steal: Whether to steal packets from the real interface
//
// @see ~VirtualInterface, ~IVirtualInterface
//
simple VirtualTunnel
{
    parameters:
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
