//
// Copyright (C) 2020 Opensim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ethernet;

import inet.common.SimpleModule;

//
// A simple Ethernet hub device that operates at the physical layer. It connects
// multiple Ethernet devices together in a star topology while creating a single
// collision domain.
//
// The hub works by broadcasting all incoming signals to all connected ports except
// the one they arrived on, effectively simulating the shared medium behavior of
// early Ethernet networks. This includes not only complete frames but also partial
// transmissions, collisions, and other electrical signals.
//
// Note that this model has limitations in accurately representing frame truncations
// that occur when links go down or come up during an ongoing transmission.
//
// @see ~EthernetSwitch, ~EthernetHost
//
simple EthernetHub extends SimpleModule
{
    parameters:
        @networkNode();
        @class(::inet::physicallayer::WireJunction);
        @labels(node,ethernet-node);
        @display("i=device/hub;bgb=155,104");
        @signal[packetReceived](type=cMessage);
        @statistic[packetReceived](title="packets"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        inout ethg[] @labels(EtherFrame-conn);
}

