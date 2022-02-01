//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet pushers. A packet pusher
// connects one input to one output. Packets can be pushed into its input
// and it pushed packets into its output.
//
moduleinterface IPacketPusher extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @display("i=block/star");
}
