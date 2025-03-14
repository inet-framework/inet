//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for tunnel network interfaces that create virtual connections
// between network nodes. Tunnel interfaces encapsulate packets from one protocol
// within another protocol, allowing traffic to pass through networks that wouldn't
// normally support the encapsulated protocol. They are commonly used for implementing
// VPNs, network virtualization, and protocol transitions.
//
// @see TunInterface, Tun
//
moduleinterface ITunnelInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
}
