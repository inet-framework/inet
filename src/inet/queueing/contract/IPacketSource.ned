//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet source modules which are both
// active and passive simultaneously.
//
// @see ~IPassivePacketSource, ~IActivePacketSource
//
moduleinterface IPacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(push,pull);
}
