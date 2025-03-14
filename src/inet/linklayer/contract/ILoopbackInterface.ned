//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for loopback network interfaces that loop back packets from the
// network layer to itself without any packet loss or transformation. Loopback interfaces
// enable applications and protocols to communicate within the same node, similar to
// the loopback interface (lo0, localhost) in real operating systems.
//
// @see LoopbackInterface, Loopback
//
moduleinterface ILoopbackInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
}
