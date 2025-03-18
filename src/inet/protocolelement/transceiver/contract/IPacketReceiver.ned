//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.transceiver.contract;

//
// Defines a module interface for packet receiver implementations. Packet
// receivers are responsible for receiving signals from a channel and converting
// them back to packets. They handle signal reception, decoding, and delivery
// to upper protocol layers.
//
moduleinterface IPacketReceiver
{
    parameters:
        @display("i=block/wrx");
    gates:
        input in;
        output out;
}
