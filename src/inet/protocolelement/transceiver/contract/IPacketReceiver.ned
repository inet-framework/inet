//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver.contract;

moduleinterface IPacketReceiver
{
    parameters:
        @display("i=block/wrx");
    gates:
        input in;
        output out;
}

