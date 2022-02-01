//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.contract;

moduleinterface IPacketTransmitter
{
    parameters:
        @display("i=block/wtx");
    gates:
        input in;
        output out;
}
