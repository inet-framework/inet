//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet sink modules which are both
// passive and active simultaneously.
//
// @see ~IPassivePacketSink, ~IActivePacketSink
//
moduleinterface IPacketSink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in @labels(push,pull);
}
