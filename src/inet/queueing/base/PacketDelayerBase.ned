//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

simple PacketDelayerBase extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        int schedulingPriority = default(0);
        bool scheduleZeroDelay = default(true);
        @class(PacketDelayerBase);
        @display("i=block/delay");
    gates:
        input in @labels(push);
        output out @labels(push);
}
