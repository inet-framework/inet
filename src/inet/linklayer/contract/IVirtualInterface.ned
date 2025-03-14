//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for virtual network interfaces that can be attached to real network
// interfaces, allowing multiple logical interfaces to share the same physical connectivity.
// Virtual interfaces enable network virtualization, traffic isolation, and specialized
// handling of different traffic types, similar to virtual interfaces in operating systems
// (e.g., eth0.1, vlan0).
//
// @see VirtualInterface, VirtualTunnel
//
moduleinterface IVirtualInterface extends INetworkInterface
{
    parameters:
        @display("i=block/tunnel");
}
