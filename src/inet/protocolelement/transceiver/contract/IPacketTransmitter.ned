//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.transceiver.contract;

//
// Defines a module interface for packet transmitter implementations. Packet
// transmitters are responsible for converting packets to signals and transmitting
// them over a channel. They handle signal encoding, transmission timing, and
// channel access coordination.
//
moduleinterface IPacketTransmitter
{
    parameters:
        @display("i=block/wtx");
    gates:
        input in;
        output out;
}
