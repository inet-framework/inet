//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface is implemented by packet scheduler modules. A packet
// scheduler is a passive module which has multiple active inputs and one
// passive output. Packets pulled from the passive output are provided by
// one of the inputs without any delay and reordering.
//
moduleinterface IPacketScheduler extends IPassivePacketSource
{
    parameters:
        @display("i=block/join");
    gates:
        input in[] @labels(pull);
}
