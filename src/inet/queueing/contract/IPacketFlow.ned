//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet flows. A packet flow connects
// one input to one output. It can operate in both active and passive mode.
// Packets can be pushed into its input or packets can be pulled from its
// output. Packets are never dropped but the contents and/or attached meta
// information may be altered.
//
moduleinterface IPacketFlow extends IPacketSink, IPacketSource
{
    parameters:
        @omittedTypename(OmittedPacketFlow);
        @display("i=block/star");
}
