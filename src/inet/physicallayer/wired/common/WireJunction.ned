//
// Copyright (C) 2020 Opensim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.common;

//
// Models a generic wiring hub.
//
// The model simply broadcasts messages (packets, frames, signals)
// received on one port on all other ports, mimicking the propagation
// of electrical signals. Messages are not interpreted by the model
// in any way, so this module is protocol agnostic.
//
// Note that no special attention is made to appropriately model
// frame truncations that occur when the link breaks, or comes
// back up while a transmission is underway. If you need
// to precisely model what happens when the link state changes,
// you cannot use this module.
//
simple WireJunction
{
    parameters:
        @wireJunction;
        @display("i=misc/node_vs");
        @signal[packetReceived](type=cPacket);
        @statistic[packetReceived](title="packets"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        inout port[];
}
