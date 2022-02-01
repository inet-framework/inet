//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet gate. A packet gate connects
// one input to one output. It can operate in both active and passive mode.
// Packets can be pushed into its input or packets can be pulled from its
// output.
//
moduleinterface IPacketGate extends IPacketSink, IPacketSource
{
    parameters:
        @omittedTypename(OmittedPacketGate);
        @display("i=block/star");
}
