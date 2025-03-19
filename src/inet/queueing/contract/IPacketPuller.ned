//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet pullers. A packet puller
// connects one input to one output. Packets can be pulled from its output
// and it pulls packets from its input.
//
moduleinterface IPacketPuller extends IActivePacketSink, IPassivePacketSource
{
    parameters:
        @display("i=block/star");
}
