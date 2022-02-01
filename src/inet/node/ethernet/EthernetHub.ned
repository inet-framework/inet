//
// Copyright (C) 2020 Opensim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ethernet;

//
// An Ethernet hub model.
//
// The model simply broadcasts frames (and transmission updates
// representing aborted frames, etc.) received on one port on all
// other ports, mimicking the propagation of electrical signals.
//
// Note that no special attention is made to appropriately model
// frame truncations that occur when the link breaks, or comes
// back up while a transmission is underway. If you need
// to precisely model what happens when the link state changes,
// you cannot use this module.
//
simple EthernetHub
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

